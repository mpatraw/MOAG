
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

static std::unique_ptr<m::server> server;
static m::land main_land;

static inline void broadcast_packed_land_chunk(m::moag *m, int x, int y, int w, int h) {
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > g_land_width) { w = g_land_width - x; }
    if (y + h > g_land_height) {
        h = g_land_height - y;
    }
    if (w <= 0 || h <= 0 || x + w > g_land_width || y + h > g_land_height) {
        return;
    }

    uint8_t *land = (uint8_t *)malloc(w * h);
    int i = 0;
    for (int yy = y; yy < h + y; ++yy) {
        for (int xx = x; xx < w + x; ++xx) {
            land[i] = main_land.is_dirt(xx, yy);
            i++;
        }
    }

    size_t packed_data_len = 0;
    uint8_t *packed_data = rlencode(land, w * h, &packed_data_len);
    free(land);

    m::packet p;

    p << static_cast<uint8_t>(PACKED_LAND_CHUNK);
    p << static_cast<uint16_t>(x);
    p << static_cast<uint16_t>(y);
    p << static_cast<uint16_t>(w);
    p << static_cast<uint16_t>(h);
    p.load(packed_data, packed_data_len);

    server->broadcast(p);

    free(packed_data);
}

static inline void broadcast_tank_chunk(m::moag *m, int action, int id) {
    m::packet p;
    
    p << static_cast<uint8_t>(TANK_CHUNK);
    p << static_cast<uint8_t>(action);
    p << static_cast<uint8_t>(id);
    p << static_cast<uint16_t>(m->players[id].tank.x);
    p << static_cast<uint16_t>(m->players[id].tank.y);
    if (m->players[id].tank.facingleft) {
        p << static_cast<uint8_t>(-m->players[id].tank.angle);
    } else {
        p << static_cast<uint8_t>(m->players[id].tank.angle);
    }

    if (action == SPAWN || action == KILL) {
        server->broadcast(p);
    } else {
        server->broadcast(p, false);
    }
}

static inline void broadcast_bullet_chunk(m::moag *m, int action, int id) {
    m::packet p;
    
    p << static_cast<uint8_t>(BULLET_CHUNK);
    p << static_cast<uint8_t>(action);
    p << static_cast<uint8_t>(id);
    p << static_cast<uint16_t>(m->bullets[id].x);
    p << static_cast<uint16_t>(m->bullets[id].y);

    if (action == SPAWN || action == KILL) {
        server->broadcast(p);
    } else {
        server->broadcast(p, false);
    }
}

static inline void broadcast_crate_chunk(m::moag *m, int action) {
    m::packet p;
    
    p << static_cast<uint8_t>(CRATE_CHUNK);
    p << static_cast<uint8_t>(action);
    p << static_cast<uint16_t>(m->crate.x);
    p << static_cast<uint16_t>(m->crate.y);

    if (action == SPAWN || action == KILL) {
        server->broadcast(p);
    } else {
        server->broadcast(p, false);
    }
}

static inline void broadcast_chat(int id, char action, const char *msg, size_t len) {
    m::packet p;
    
    p << static_cast<uint8_t>(SERVER_MSG_CHUNK);
    p << static_cast<uint8_t>(id);
    p << static_cast<uint8_t>(action);
    p << msg;

    server->broadcast(p);
}


static void kill_tank(m::moag *m, int id) {
    m->players[id].tank.x = -30;
    m->players[id].tank.y = -30;
    m->players[id].spawn_timer = g_respawn_time;
    broadcast_tank_chunk(m, KILL, id);
}

static void explode(m::moag *m, int x, int y, int rad, char type) {
    for (int iy = -rad; iy <= rad; iy++) {
        for (int ix = -rad; ix <= rad; ix++) {
            if (std::pow(ix, 2) + std::pow(iy, 2) < std::pow(rad, 2)) {
                if (type == E_DIRT) {
                    main_land.set_dirt(x/10 + ix, y/10 + iy);
                } else {
                    main_land.set_air(x/10 + ix, y/10 + iy);
                }
            }
        }
    }
    if (type == E_EXPLODE) {
        for (int i = 0; i < g_max_players; i++) {
			const auto &t = m->players[i].tank;
            if (m->players[i].connected &&
				std::pow(t.x - x, 2) + std::pow(t.y - 3 - y, 2) < std::pow(rad * 10 + 4, 2)) {
                kill_tank(m, i);
            }
        }
    }

    broadcast_packed_land_chunk(m, x/10 - rad, y/10 - rad, rad * 2, rad * 2);
}

static void spawn_tank(m::moag *m, int id) {
    m->players[id].connected = true;
    m->players[id].spawn_timer = 0;
    m->players[id].ladder_timer = g_ladder_time;
    m->players[id].ladder_count = 3;
    m->players[id].kleft = false;
    m->players[id].kright = false;
    m->players[id].kup = false;
    m->players[id].kdown = false;
    m->players[id].kfire = false;
    m->players[id].tank.x = rand() % g_land_width * 10;
    m->players[id].tank.y = 60;
    m->players[id].tank.velx = 0;
    m->players[id].tank.vely = 0;
    m->players[id].tank.angle = 35;
    m->players[id].tank.facingleft = 0;
    m->players[id].tank.power = 0;
    m->players[id].tank.bullet = MISSILE;
    m->players[id].tank.num_burst = 1;
    explode(m, m->players[id].tank.x, m->players[id].tank.y - 12, 12, E_SAFE_EXPLODE);
    broadcast_tank_chunk(m, SPAWN, id);
}

static void spawn_client(m::moag *m, int id) {
    sprintf(m->players[id].name,"p%d",id);
    char notice[64] = "  ";
    strcat(notice, m->players[id].name);
    strcat(notice, " has connected");
    broadcast_chat(-1, SERVER_NOTICE, notice, strlen(notice) + 1);

    spawn_tank(m, id);
    broadcast_packed_land_chunk(m, 0, 0, g_land_width, g_land_height);
    broadcast_chat(id, NAME_CHANGE,m->players[id].name, strlen(m->players[id].name) + 1);
    if (m->crate.active) {
        broadcast_crate_chunk(m, SPAWN);
    }

    for (int i = 0; i < g_max_players; ++i) {
        if (m->players[i].connected) {
            broadcast_tank_chunk(m, SPAWN, i);
            broadcast_chat(i, NAME_CHANGE, m->players[i].name, strlen(m->players[i].name) + 1);
        }
    }
}

static void disconnect_client(m::moag *m, int id) {
    m->players[id].connected = 0;
    broadcast_tank_chunk(m, KILL, id);
}

static void launch_ladder(m::moag *m, int x, int y) {
    int i = 0;
    while (m->bullets[i].active) {
        if (++i >= g_max_bullets) {
            return;
        }
    }
    m->bullets[i].active = g_ladder_length;
    m->bullets[i].type = LADDER;
    m->bullets[i].x = x;
    m->bullets[i].y = y;
    m->bullets[i].velx = 0;
    m->bullets[i].vely = -1;
    broadcast_bullet_chunk(m, SPAWN, i);
}

static void fire_bullet(m::moag *m, int origin, char type, int x, int y, int vx, int vy) {
    int i = 0;
    while (m->bullets[i].active) {
        if (++i >= g_max_bullets) {
            return;
        }
    }
    m->bullets[i].active = 1;
	m->bullets[i].origin = origin;
    m->bullets[i].type = type;
    m->bullets[i].x = x;
    m->bullets[i].y = y;
    m->bullets[i].velx = vx;
    m->bullets[i].vely = vy;
    broadcast_bullet_chunk(m, SPAWN, i);
}

static void fire_bullet_ang(m::moag *m, int origin, char type, int x, int y, int angle, int vel) {
    fire_bullet(m, origin, type,
		static_cast<int>(x + 5 * cos(radians(angle))),
		static_cast<int>(y - 5 * sin(radians(angle))),
		static_cast<int>(vel * cos(radians(angle)) / 10),
		static_cast<int>(-vel * sin(radians(angle)) / 10));
}

static void tank_update(m::moag *m, int id) {
    m::tank *t = &m->players[id].tank;

    if (!m->players[id].connected) {
        return;
    }

    if (m->players[id].spawn_timer > 0) {
        if (--m->players[id].spawn_timer <= 0) {
            spawn_tank(m, id);
        }
        return;
    }

    bool moved = false;
    bool grav = true;
    if (m->players[id].ladder_timer >= 0 && (!m->players[id].kleft || !m->players[id].kright)) {
        m->players[id].ladder_timer = g_ladder_time;
    }

    if (m->players[id].kleft && m->players[id].kright) {
        if (m->players[id].ladder_timer > 0) {
            if (main_land.is_dirt(t->x/10, t->y/10 + 1)) {
                m->players[id].ladder_timer--;
            } else {
                m->players[id].ladder_timer = g_ladder_time;
            }
        } else if (m->players[id].ladder_timer == 0) {
            if (m->players[id].ladder_count > 0) {
                launch_ladder(m, t->x, t->y);
                m->players[id].ladder_count--;
            }
            m->players[id].ladder_timer = g_ladder_time;
        }
    } else if (m->players[id].kleft) {
        t->facingleft = 1;
        if (main_land.is_air(t->x/10 - 1, t->y/10) && t->x >= 10) {
            t->x -= 10;
        } else if (main_land.is_air(t->x/10 - 1, t->y/10 - 1) && t->x >= 10) {
            t->x -= 10;
            t->y -= 10;
        } else if (main_land.is_air(t->x/10, t->y/10 - 1) ||
                 main_land.is_air(t->x/10, t->y/10 - 2) ||
                 main_land.is_air(t->x/10, t->y/10 - 3)) {
            grav = false;
            t->y -= 10;
        } else {
            grav = false;
        }
        moved = true;
    } else if (m->players[id].kright) {
        t->facingleft = 0;
        if (main_land.is_air(t->x/10 + 1, t->y/10) && t->x/10 < g_land_width - 10) {
            t->x += 10;
        } else if (main_land.is_air(t->x/10 + 1, t->y/10 - 1) && t->x < g_land_width - 10) {
            t->x += 10;
            t->y -= 10;
        } else if (main_land.is_air(t->x/10, t->y/10 - 1) ||
                 main_land.is_air(t->x/10, t->y/10 - 2) ||
                 main_land.is_air(t->x/10, t->y/10 - 3)) {
            grav = false;
            t->y -= 10;
        } else {
            grav = false;
        }
        moved = true;
    }

    // Physics
    if (t->y / 10 < 20) {
        t->y = 200;
        moved = true;
    }

    if (grav) {
        if (main_land.is_air(t->x/10, t->y/10 + 1)) {
            t->y += 10;
            moved = true;
        }
    }

    if (abs(t->x - m->crate.x) < 14 && abs(t->y - m->crate.y) < 14) {
        m->players[id].ladder_timer = g_ladder_time;
        if (m->crate.type == TRIPLER) {
            t->num_burst *= 3;
        } else {
            t->bullet = m->crate.type;
        }
        m->crate.active = false;
        broadcast_crate_chunk(m, KILL);
        char notice[64] = "* ";
        strcat(notice, m->players[id].name);
        strcat(notice, " got ");
        switch (m->crate.type) {
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
        broadcast_chat(-1, SERVER_NOTICE, notice, strlen(notice) + 1);
    }

    // Aim
    if (m->players[id].kup && t->angle < 90) {
        t->angle++;
        moved = true;
    } else if (m->players[id].kdown && t->angle > 1) {
        t->angle--;
        moved = true;
    }

    // Fire
    if (t->power) {
        int start_angle = t->facingleft ? 180 - t->angle : t->angle;
        fire_bullet_ang(m, id, t->bullet, t->x, t->y - 70, start_angle, t->power);
        t->bullet = MISSILE;
        t->power = 0;
    }

    if (moved) {
        broadcast_tank_chunk(m, MOVE, id);
    }
}

static void bullet_detonate(m::moag *m, int id) {
    m::bullet *b = &m->bullets[id];

    switch (b->type) {
        case MISSILE:
            explode(m, b->x, b->y, 12, E_EXPLODE);
            break;

        case SHOTGUN:
            break;

        case BABY_NUKE:
            explode(m, b->x, b->y, 55, E_EXPLODE);
            break;

        case NUKE:
            explode(m, b->x, b->y, 150, E_EXPLODE);
            break;

        case DIRT:
            explode(m, b->x, b->y, 55, E_DIRT);
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
        broadcast_bullet_chunk(m, KILL, id);
    }
}

static void bullet_update(m::moag *m, int id) {
    m::bullet *b = &m->bullets[id];

	if (!b->active) {
		return;
	}

    b->x += b->velx;
    b->y += b->vely;
    b->vely += g_gravity;

    if (main_land.is_dirt(b->x / 10, b->y / 10)) {
        bullet_detonate(m, id);
        return;
    }

	for (int i = 0; i < g_max_players; i++) {
		if (!m->players[i].connected) {
			continue;
		}
        const auto &t = m->players[i].tank;
        auto dx = t.x - b->x;
        auto dy = t.y - b->y;
        if (dx * dx + dy * dy < 90 && i != b->origin) {
            bullet_detonate(m, id);
            return;
        }
    }

    if (b->active) {
        broadcast_bullet_chunk(m, MOVE, id);
    }
}

static void crate_update(m::moag *m) {
    if (!m->crate.active) {
        m->crate.active = true;
        m->crate.x = rand() % g_land_width * 10;
        m->crate.y = 300;
        explode(m, m->crate.x, m->crate.y - 120, 12, E_SAFE_EXPLODE);

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
             if ((r -= PBABYNUKE) < 0)   m->crate.type = BABY_NUKE;
        else if ((r -= PNUKE) < 0)       m->crate.type = NUKE;
        else if ((r -= PSUPERDIRT) < 0)  m->crate.type = SUPER_DIRT;
        else if ((r -= PLIQUIDDIRT) < 0) m->crate.type = LIQUID_DIRT;
        else if ((r -= PCOLLAPSE) < 0)   m->crate.type = COLLAPSE;
        else if ((r -= PBOUNCER) < 0)    m->crate.type = BOUNCER;
        else if ((r -= PTUNNELER) < 0)   m->crate.type = TUNNELER;
        else if ((r -= PMIRV) < 0)       m->crate.type = MIRV;
        else if ((r -= PCLUSTER) < 0)    m->crate.type = CLUSTER_BOMB;
        else if ((r -= PCLUSTERB) < 0)   m->crate.type = CLUSTER_BOUNCER;
        else if ((r -= PSHOTGUN) < 0)    m->crate.type = SHOTGUN;
        else if ((r -= PTRIPLER) < 0)    m->crate.type = TRIPLER;
        else                             m->crate.type = DIRT;
        broadcast_crate_chunk(m, SPAWN);
    }

    if (main_land.is_air(m->crate.x / 10, (m->crate.y + 1) / 10)) {
        m->crate.y += 10;
        broadcast_crate_chunk(m, MOVE);
    }
}

static void step_game(m::moag *m) {
    crate_update(m);
    for (int i = 0; i < g_max_players; i++) {
        tank_update(m, i);
    }
    for (int i = 0; i < g_max_bullets; i++) {
        bullet_update(m, i);
    }
    m->frame += 1;
}

static void handle_msg(m::moag *m, int id, const char* msg, int len) {
    if (msg[0] == '/' && msg[1] == 'n' && msg[2] == ' ') {
        char notice[64] = "  ";
        strcat(notice, m->players[id].name);
        len -= 3;
        if (len > 15) {
            len = 15;
        }
        for (int i = 0; i < len; i++) {
            m->players[id].name[i] = msg[i + 3];
        }
        m->players[id].name[len] = '\0';
        strcat(notice, " is now known as ");
        strcat(notice, m->players[id].name);
        broadcast_chat(id, NAME_CHANGE, m->players[id].name, strlen(m->players[id].name) + 1);
        broadcast_chat(-1, SERVER_NOTICE, notice, strlen(notice) + 1);
    } else {
        broadcast_chat(id, CHAT, msg, len + 1);
    }
}

static void init_game(m::moag *m) {
    for (int i = 0; i < g_max_players; i++) {
        m->players[i].connected = 0;
    }
    for (int i = 0; i < g_max_bullets; i++) {
        m->bullets[i].active = 0;
    }
    m->crate.active = false;
    m->frame = 1;

    for (int y = 0; y < g_land_height; ++y) {
        for (int x = 0; x < g_land_width; ++x) {
            if (y < g_land_height / 3) {
                main_land.set_air(x, y);
            } else {
                main_land.set_dirt(x, y);
            }
        }
    }
}

static void process_packet(m::moag *m, m::packet &p, uint8_t type, int id) {
    switch (type) {
        case INPUT_CHUNK: {
            uint8_t key;
            uint16_t ms;
            p >> key >> ms;
            switch (key) {
                case KLEFT_PRESSED:   m->players[id].kleft = true; break;
                case KLEFT_RELEASED:  m->players[id].kleft = false; break;
                case KRIGHT_PRESSED:  m->players[id].kright = true; break;
                case KRIGHT_RELEASED: m->players[id].kright = false; break;
                case KUP_PRESSED:     m->players[id].kup = true; break;
                case KUP_RELEASED:    m->players[id].kup = false; break;
                case KDOWN_PRESSED:   m->players[id].kdown = true; break;
                case KDOWN_RELEASED:  m->players[id].kdown = false; break;
                case KFIRE_PRESSED:   m->players[id].kfire = true; break;
                case KFIRE_RELEASED: {
                    m->players[id].tank.power = ms;
                    m->players[id].kfire = false;
                    auto p = &m->players[id].tank.power;
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

        case CLIENT_MSG_CHUNK: {
            std::string msg;
            p >> msg;
            handle_msg(m, id, msg.c_str(), msg.length());
            break;
        }

        default: break;
    }
}

void server_main() {
    server.reset(new m::server);

    m::moag moag;
    init_game(&moag);

    while (true) {
        while (true) {
            int id;
            auto packet = server->recv(id);
            if (packet.empty()) {
                break;
            }

            uint8_t type;
            packet >> type;
            if (type == m::packet_type_connection) {
                spawn_client(&moag, id);
            } else if (type == m::packet_type_disconnection) {
                disconnect_client(&moag, id);
            } else {
                process_packet(&moag, packet, type, id);
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        step_game(&moag);
    }
}
