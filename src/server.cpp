
#include <cmath>
#include <cstring>

#include <chrono>
#include <iostream>
#include <thread>

#include "moag.hpp"

enum {
    MISSILE,
    BABY_NUKE,
    NUKE,
    DIRT,
    SUPER_DIRT,
    COLLAPSE,
    LIQUID_DIRT,
    BOUNCER,
    TUNNELER,
    LADDER,
    MIRV,
    MIRV_WARHEAD,
    CLUSTER_BOMB,
    CLUSTER_BOUNCER,
    SHOTGUN,
    LIQUID_DIRT_WARHEAD,
    TRIPLER,
};

enum {
    E_EXPLODE,
    E_DIRT,
    E_SAFE_EXPLODE,
    E_COLLAPSE
};

static std::unique_ptr<m::network_manager> server;
static m::land main_land;
static m::physics physics{main_land};

static m::player players[g_max_players];
static m::bullet bullets[g_max_bullets];
static m::crate crate;

static inline void broadcast_packed_land_chunk(int x, int y, int w, int h) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > m::land_width) { w = m::land_width - x; }
    if (y + h > m::land_height) {
        h = m::land_height - y;
    }
    if (w <= 0 || h <= 0 || x + w > m::land_width || y + h > m::land_height) {
        return;
    }

    std::vector<uint8_t> land(w * h);
    int i = 0;
    for (int yy = y; yy < h + y; ++yy) {
        for (int xx = x; xx < w + x; ++xx) {
            land[i] = main_land.is_dirt(xx, yy);
            i++;
        }
    }

    m::message msg{new m::land_message_def{
        static_cast<uint16_t>(x),
        static_cast<uint16_t>(y),
        static_cast<uint16_t>(w),
        static_cast<uint16_t>(h), land}};
    server->send(msg);
}

static inline void broadcast_tank_chunk(m::entity_op_type op, uint8_t id) {
    m::message msg{new m::tank_message_def{
        op,
        players[id].the_tank.facingleft ? m::tank_facing_type::left : m::tank_facing_type::right,
        id,
        static_cast<uint8_t>(players[id].the_tank.angle),
        static_cast<uint16_t>(players[id].the_tank.x),
        static_cast<uint16_t>(players[id].the_tank.y)}};

    server->send(msg);
}

static inline void broadcast_bullet_chunk(m::entity_op_type op, uint8_t id) {
    m::message msg{new m::bullet_message_def{ 
        op,
        id,
        static_cast<uint16_t>(bullets[id].x),
        static_cast<uint16_t>(bullets[id].y)}};

    server->send(msg);
}

static inline void broadcast_crate_chunk(m::entity_op_type op) {
    m::message msg{new m::crate_message_def{
        op,
        static_cast<uint16_t>(crate.get_body().x),
        static_cast<uint16_t>(crate.get_body().y)}};

    server->send(msg);
}

static inline void broadcast_chat(m::message_op_type op, uint8_t id, const char *str) {
    m::message msg{new m::message_message_def{op, id, str}};
    server->send(msg);
}


static void kill_tank(uint8_t id) {
    players[id].the_tank.x = -30;
    players[id].the_tank.y = -30;
    players[id].spawn_timer = g_respawn_time;
    broadcast_tank_chunk(m::entity_op_type::kill, id);
}

static void explode(int x, int y, int rad, char type) {
    for (int iy = -rad; iy <= rad; iy++) {
        for (int ix = -rad; ix <= rad; ix++) {
            if (std::pow(ix, 2) + std::pow(iy, 2) < std::pow(rad, 2)) {
                if (type == E_DIRT) {
                    main_land.set_dirt(x + ix, y + iy);
                } else {
                    main_land.set_air(x + ix, y + iy);
                }
            }
        }
    }
    if (type == E_EXPLODE) {
        for (int i = 0; i < g_max_players; i++) {
            const auto &t = players[i].the_tank;
            if (players[i].connected &&
                std::pow((t.x - x), 2) +
                std::pow((t.y - 3 - y), 2) <
                std::pow(rad + 4, 2)) {
                kill_tank(i);
            }
        }
    }

    broadcast_packed_land_chunk(x - rad, y - rad, rad * 2, rad * 2);
}

static void spawn_tank(int id) {
    players[id].connected = true;
    players[id].spawn_timer = 0;
    players[id].kleft = false;
    players[id].kright = false;
    players[id].kup = false;
    players[id].kdown = false;
    players[id].kfire = false;
    players[id].the_tank.x = rand() % m::land_width;
    players[id].the_tank.y = 60;
    players[id].the_tank.velx = 0;
    players[id].the_tank.vely = 0;
    players[id].the_tank.angle = 35;
    players[id].the_tank.facingleft = 0;
    players[id].the_tank.power = 0;
    players[id].the_tank.bullet = MISSILE;
    players[id].the_tank.num_burst = 1;
    explode(players[id].the_tank.x, (players[id].the_tank.y - 12), 12, E_SAFE_EXPLODE);
    broadcast_tank_chunk(m::entity_op_type::spawn, id);
}

static void spawn_client(int id) {
    sprintf(players[id].name,"p%d",id);
    char notice[64] = "  ";
    strcat(notice, players[id].name);
    strcat(notice, " has connected");
    broadcast_chat(m::message_op_type::server_notice, -1, notice);

    spawn_tank(id);
    broadcast_packed_land_chunk(0, 0, m::land_width, m::land_height);
    broadcast_chat(m::message_op_type::name_change, id, players[id].name);
    if (crate.alive) {
        broadcast_crate_chunk(m::entity_op_type::spawn);
    }

    for (int i = 0; i < g_max_players; ++i) {
        if (players[i].connected) {
            broadcast_tank_chunk(m::entity_op_type::spawn, i);
            broadcast_chat(m::message_op_type::name_change, i, players[i].name);
        }
    }
}

static void disconnect_client(int id) {
    players[id].connected = 0;
    broadcast_tank_chunk(m::entity_op_type::kill, id);
}

static void fire_bullet(int origin, char type, int x, int y, int vx, int vy) {
    int i = 0;
    while (bullets[i].active) {
        if (++i >= g_max_bullets) {
            return;
        }
    }
    bullets[i].active = 1;
    bullets[i].origin = origin;
    bullets[i].type = type;
    bullets[i].x = x;
    bullets[i].y = y;
    bullets[i].velx = vx;
    bullets[i].vely = vy;
    broadcast_bullet_chunk(m::entity_op_type::spawn, i);
}

static void fire_bullet_ang(int origin, char type, int x, int y, int angle, int vel) {
    fire_bullet(origin, type,
        static_cast<int>(x + 5 * std::cos(radians(angle))),
        static_cast<int>(y - 5 * std::sin(radians(angle))),
        static_cast<int>(vel * std::cos(radians(angle))),
        static_cast<int>(-vel * std::sin(radians(angle))));
}

static void tank_update(int id) {
    m::tank *t = &players[id].the_tank;

    if (!players[id].connected) {
        return;
    }

    if (players[id].spawn_timer > 0) {
        if (--players[id].spawn_timer <= 0) {
            spawn_tank(id);
        }
        return;
    }

    bool moved = false;
    bool grav = true;

    if (players[id].kleft) {
        t->facingleft = 1;
        if (main_land.is_air(t->x - 1, t->y) && t->x >= 1) {
            t->x -= 1;
        } else if (main_land.is_air(t->x - 1, t->y - 1) && t->x >= 1) {
            t->x -= 1;
            t->y -= 1;
        } else if (main_land.is_air(t->x, t->y - 1) ||
                 main_land.is_air(t->x, t->y - 2) ||
                 main_land.is_air(t->x, t->y - 3)) {
            grav = false;
            t->y -= 1;
        } else {
            grav = false;
        }
        moved = true;
    } else if (players[id].kright) {
        t->facingleft = 0;
        if (main_land.is_air(t->x + 1, t->y) && t->x < m::land_width - 1) {
            t->x += 1;
        } else if (main_land.is_air(t->x + 1, t->y - 1) && t->x < m::land_width - 1) {
            t->x += 1;
            t->y -= 1;
        } else if (main_land.is_air(t->x, t->y - 1) ||
                 main_land.is_air(t->x, t->y - 2) ||
                 main_land.is_air(t->x, t->y - 3)) {
            grav = false;
            t->y -= 1;
        } else {
            grav = false;
        }
        moved = true;
    }

    // Physics
    if (t->y < 20) {
        t->y = 20;
        moved = true;
    }

    if (grav) {
        if (main_land.is_air(t->x, t->y + 1)) {
            t->y += 1;
            moved = true;
        }
    }

    if (std::abs(t->x - crate.get_body().x) < 14 && std::abs(t->y - crate.get_body().y) < 14) {
        if (crate.type == TRIPLER) {
            t->num_burst *= 3;
        } else {
            t->bullet = crate.type;
        }
        crate.alive = false;
        broadcast_crate_chunk(m::entity_op_type::kill);
        char notice[64] = "* ";
        strcat(notice, players[id].name);
        strcat(notice, " got ");
        switch (crate.type) {
            case  0: strcat(notice, "Missile"); break;
            case  1: strcat(notice, "Baby Nuke"); break;
            case  2: strcat(notice, "Nuke"); break;
            case  3: strcat(notice, "Dirtball"); break;
            case  4: strcat(notice, "Super Dirtball"); break;
            case  5: strcat(notice, "Collapse"); break;
            case  6: strcat(notice, "Liquid Dirt"); break;
            case  7: strcat(notice, "Bouncer"); break;
            case  8: strcat(notice, "Tunneler"); break;
            case 10: strcat(notice, "MIRV"); break;
            case 12: strcat(notice, "Cluster Bomb"); break;
            case 13: strcat(notice, "Cluster Bouncer"); break;
            case 14: strcat(notice, "Shotgun"); break;
            case 16: strcat(notice, "*Triple*"); break;
            default: strcat(notice, "???"); break;
        }
        broadcast_chat(m::message_op_type::server_notice, -1, notice);
    }

    // Aim
    if (players[id].kup && t->angle < 90) {
        t->angle++;
        moved = true;
    } else if (players[id].kdown && t->angle > 1) {
        t->angle--;
        moved = true;
    }

    // Fire
    if (t->power) {
        int start_angle = t->facingleft ? 180 - t->angle : t->angle;
        fire_bullet_ang(id, t->bullet, t->x, t->y - 7, start_angle, t->power / 50);
        t->bullet = MISSILE;
        t->power = 0;
    }

    if (moved) {
        broadcast_tank_chunk(m::entity_op_type::move, id);
    }
}

static void bullet_detonate(int id) {
    m::bullet *b = &bullets[id];

    switch (b->type) {
        case MISSILE:
            explode(b->x, b->y, 12, E_EXPLODE);
            break;

        case SHOTGUN:
            break;

        case BABY_NUKE:
            explode(b->x, b->y, 55, E_EXPLODE);
            break;

        case NUKE:
            explode(b->x, b->y, 150, E_EXPLODE);
            break;

        case DIRT:
            explode(b->x, b->y, 55, E_DIRT);
            break;

        case SUPER_DIRT:
            break;

        case COLLAPSE:
            break;

        case LIQUID_DIRT:
            break;

        case LIQUID_DIRT_WARHEAD:
            break;

        case BOUNCER:
            break;

        case TUNNELER:
            break;

        case LADDER:
            break;

        case MIRV:
            break;

        case MIRV_WARHEAD:
            break;

        case CLUSTER_BOMB:
            break;

        case CLUSTER_BOUNCER:
            break;

        default: break;
    }

    if (b->active >= 0) {
        b->active = 0;
        broadcast_bullet_chunk(m::entity_op_type::kill, id);
    }
}

static void bullet_update(int id) {
    m::bullet *b = &bullets[id];

    if (!b->active) {
        return;
    }

    b->x += b->velx;
    b->y += b->vely;
    b->vely += g_gravity;

    if (main_land.is_dirt(b->x, b->y)) {
        bullet_detonate(id);
        return;
    }

    for (int i = 0; i < g_max_players; i++) {
        if (!players[i].connected) {
            continue;
        }
        const auto &t = players[i].the_tank;
        auto dx = t.x - b->x;
        auto dy = t.y - b->y;
        if (dx * dx + dy * dy < 90 && i != b->origin) {
            bullet_detonate(id);
            return;
        }
    }

    if (b->active) {
        broadcast_bullet_chunk(m::entity_op_type::move, id);
    }
}

static void crate_update(float dt) {
    if (!crate.alive) {
        crate.alive = true;
        crate.get_body().x = static_cast<uint16_t>(rand() % m::land_width);
        crate.get_body().y = static_cast<uint16_t>(30);

        const int PBABYNUKE = 100;
        const int PNUKE = 20;
        const int PDIRT = 75;
        const int PSUPERDIRT = 15;
        const int PLIQUIDDIRT = 60;
        const int PCOLLAPSE = 60;
        const int PBOUNCER = 100;
        const int PTUNNELER = 75;
        const int PMIRV = 40;
        const int PCLUSTER = 60;
        const int PCLUSTERB = 10;
        const int PSHOTGUN = 100;
        const int PTRIPLER = 40;
        // add new ones here:
        const int TOTAL = PBABYNUKE + PNUKE + PDIRT + PSUPERDIRT + PLIQUIDDIRT +
                          PCOLLAPSE + PBOUNCER + PTUNNELER + PMIRV + PCLUSTER +
                          PCLUSTERB + PSHOTGUN + PTRIPLER;
        int r = rand() % TOTAL;
             if ((r -= PBABYNUKE) < 0)   crate.type = BABY_NUKE;
        else if ((r -= PNUKE) < 0)       crate.type = NUKE;
        else if ((r -= PSUPERDIRT) < 0)  crate.type = SUPER_DIRT;
        else if ((r -= PLIQUIDDIRT) < 0) crate.type = LIQUID_DIRT;
        else if ((r -= PCOLLAPSE) < 0)   crate.type = COLLAPSE;
        else if ((r -= PBOUNCER) < 0)    crate.type = BOUNCER;
        else if ((r -= PTUNNELER) < 0)   crate.type = TUNNELER;
        else if ((r -= PMIRV) < 0)       crate.type = MIRV;
        else if ((r -= PCLUSTER) < 0)    crate.type = CLUSTER_BOMB;
        else if ((r -= PCLUSTERB) < 0)   crate.type = CLUSTER_BOUNCER;
        else if ((r -= PSHOTGUN) < 0)    crate.type = SHOTGUN;
        else if ((r -= PTRIPLER) < 0)    crate.type = TRIPLER;
        else                             crate.type = DIRT;
        broadcast_crate_chunk(m::entity_op_type::spawn);
    }

    crate.physics_step(physics, dt);
    broadcast_crate_chunk(m::entity_op_type::move);
}

static void step_game(float dt) {
    crate_update(dt);
    for (int i = 0; i < g_max_players; i++) {
        tank_update(i);
    }
    for (int i = 0; i < g_max_bullets; i++) {
        bullet_update(i);
    }
}

static void handle_msg(int id, const char* msg, int len) {
    if (msg[0] == '/' && msg[1] == 'n' && msg[2] == ' ') {
        char notice[64] = "  ";
        strcat(notice, players[id].name);
        len -= 3;
        if (len > 15) {
            len = 15;
        }
        for (int i = 0; i < len; i++) {
            players[id].name[i] = msg[i + 3];
        }
        players[id].name[len] = '\0';
        strcat(notice, " is now known as ");
        strcat(notice, players[id].name);
        broadcast_chat(m::message_op_type::name_change, id, players[id].name);
        broadcast_chat(m::message_op_type::server_notice, -1, notice);
    } else {
        broadcast_chat(m::message_op_type::chat, id, msg);
    }
}

static void init_game() {
    for (int i = 0; i < g_max_players; i++) {
        players[i].connected = 0;
    }
    for (int i = 0; i < g_max_bullets; i++) {
        bullets[i].active = 0;
    }
    crate.alive = false;

    for (int y = 0; y < m::land_height; ++y) {
        for (int x = 0; x < m::land_width; ++x) {
            if (y < m::land_height / 3) {
                main_land.set_air(x, y);
            } else {
                main_land.set_dirt(x, y);
            }
        }
    }
}

static void process_packet(m::packet &p) {
    auto &msg = p.get_message();
    auto id = p.get_id();
    switch (msg.get_type()) {
        case m::message_type::input: {
            auto &input = dynamic_cast<m::input_message_def &>(msg.get_def());
            switch (input.op) {
                case m::input_op_type::start_move_left: players[id].kleft = true; break;
                case m::input_op_type::stop_move_left: players[id].kleft = false; break;
                case m::input_op_type::start_move_right: players[id].kright = true; break;
                case m::input_op_type::stop_move_right: players[id].kright = false; break;
                case m::input_op_type::start_move_up: players[id].kup = true; break;
                case m::input_op_type::stop_move_up: players[id].kup = false; break;
                case m::input_op_type::start_move_down: players[id].kdown = true; break;
                case m::input_op_type::stop_move_down: players[id].kdown = false; break;
                case m::input_op_type::start_fire: players[id].kfire = true; break;
                case m::input_op_type::stop_fire: {
                    players[id].the_tank.power = input.ms;
                    players[id].kfire = false;
                    auto p = &players[id].the_tank.power;
                    if (*p < 0) {
                        *p = 0;
                    } else if (*p > 1000) {
                        *p = 1000;
                    }
                    break;
                }
            }
            break;
        }

        case m::message_type::message: {
            auto msgmsg = dynamic_cast<m::message_message_def &>(msg.get_def());
            auto str = msgmsg.get_string();
            handle_msg(id, str.c_str(), str.length());
            break;
        }

        default: break;
    }
}

void server_main(unsigned short port) {
    server.reset(new m::network_manager{port, g_max_players});

    init_game();

    auto last = std::chrono::system_clock::now();
    while (true) {
        while (true) {
            auto packet = server->recv();
            if (packet.get_type() == m::packet_type::none) {
                break;
            }

            auto type = packet.get_type();
            if (type == m::packet_type::connection) {
                spawn_client(packet.get_id());
            } else if (type == m::packet_type::disconnection) {
                disconnect_client(packet.get_id());
            } else {
                process_packet(packet);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        auto now = std::chrono::system_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(last - now);
        step_game(diff.count() / 1000.f);
        last = now;
    }
}
