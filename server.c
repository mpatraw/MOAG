
#include "server.h"

void set_timer(struct moag *m, int frame, char type, float x, float y, float vx, float vy)
{
    int i = 0;
    while (m->timers[i].frame)
        if (++i >= MAX_TIMERS)
            return;
    m->timers[i].frame = frame;
    m->timers[i].type = type;
    m->timers[i].x = x;
    m->timers[i].y = y;
    m->timers[i].vx = vx;
    m->timers[i].vy = vy;
}

void kill_tank(struct moag *m, int id)
{
    m->players[id].tank.x = -30;
    m->players[id].tank.y = -30;
    m->players[id].spawn_timer = RESPAWN_TIME;
    broadcast_tank_chunk(m, KILL, id);
}

void explode(struct moag *m, int x, int y, int rad, char type)
{
    if (type == E_COLLAPSE)
    {
        for (int iy = -rad; iy <= rad; iy++)
            for (int ix = -rad; ix <= rad; ix++)
                if (SQ(ix) + SQ(iy) < SQ(rad) && get_land_at(m, x+ix, y+iy))
                    set_land_at(m, x + ix, y + iy, 2);
        int maxy = y + rad;
        for (int iy = -rad; iy <= rad; iy++)
        {
            for (int ix = -rad; ix <= rad; ix++)
            {
                if (get_land_at(m, x + ix, y + iy) == 2)
                {
                    int count = 0;
                    int yy;
                    for (yy = y + iy;
                         yy < LAND_HEIGHT && get_land_at(m, x + ix, yy) != 1;
                         yy++)
                    {
                        if (get_land_at(m, x + ix, yy) == 2)
                        {
                            count++;
                            set_land_at(m, x + ix, yy, 0);
                        }
                    }
                    for (; count > 0; count--)
                        set_land_at(m, x + ix, yy - count, 1);
                    if (yy > maxy)
                        maxy = yy;
                }
            }
        }
        broadcast_packed_land_chunk(m, x - rad, y - rad, rad * 2, maxy - (y - rad));
        return;
    }
    char p = type == E_DIRT ? 1 : 0;
    for (int iy = -rad; iy <= rad; iy++)
        for (int ix = -rad; ix <= rad; ix++)
            if (SQ(ix) + SQ(iy) < SQ(rad))
                set_land_at(m, x + ix, y + iy, p);
    if (type == E_EXPLODE)
        for (int i = 0; i < MAX_PLAYERS; i++)
            if (m->players[i].connected &&
                SQ(m->players[i].tank.x - x) + SQ(m->players[i].tank.y - 3 - y) < SQ(rad + 4))
                kill_tank(m, i);

    broadcast_packed_land_chunk(m, x - rad, y - rad, rad * 2, rad * 2);
}

void spawn_tank(struct moag *m, int id)
{
    m->players[id].connected = true;
    m->players[id].spawn_timer = 0;
    m->players[id].ladder_timer = LADDER_TIME;
    m->players[id].ladder_count = 3;
    m->players[id].kleft = false;
    m->players[id].kright = false;
    m->players[id].kup = false;
    m->players[id].kdown = false;
    m->players[id].kfire = false;
    m->players[id].tank.x = rng_range(&m->rng, 20, LAND_WIDTH - 20);
    m->players[id].tank.y = 60;
    m->players[id].tank.obj.pos = VEC2(m->players[id].tank.x, m->players[id].tank.y);
    m->players[id].tank.obj.vel = VEC2(0, 0);
    m->players[id].tank.angle = 35;
    m->players[id].tank.facingleft = 0;
    m->players[id].tank.power = 0;
    m->players[id].tank.bullet = MISSILE;
    m->players[id].tank.num_burst = 1;
    explode(m, m->players[id].tank.x, m->players[id].tank.y - 12, 12, E_SAFE_EXPLODE);
    broadcast_tank_chunk(m, SPAWN, id);
}

void spawn_client(struct moag *m, int id)
{
    sprintf(m->players[id].name,"p%d",id);
    char notice[64] = "  ";
    strcat(notice, m->players[id].name);
    strcat(notice, " has connected");
    broadcast_chat(-1, SERVER_NOTICE, notice, strlen(notice) + 1);

    spawn_tank(m, id);
    broadcast_packed_land_chunk(m, 0, 0, LAND_WIDTH, LAND_HEIGHT);
    broadcast_chat(id, NAME_CHANGE,m->players[id].name, strlen(m->players[id].name) + 1);
    if (m->crate.active)
        broadcast_crate_chunk(m, SPAWN);

    for (int i = 0; i < MAX_PLAYERS; ++i)
    {
        if (m->players[i].connected)
        {
            broadcast_tank_chunk(m, SPAWN, i);
            broadcast_chat(i, NAME_CHANGE, m->players[i].name, strlen(m->players[i].name) + 1);
        }
    }
}

void disconnect_client(struct moag *m, int id)
{
    m->players[id].connected = 0;
    broadcast_tank_chunk(m, KILL, id);
}

void launch_ladder(struct moag *m, int x, int y)
{
    int i = 0;
    while (m->bullets[i].active)
        if (++i >= MAX_BULLETS)
            return;
    m->bullets[i].active = LADDER_LENGTH;
    m->bullets[i].type = LADDER;
    m->bullets[i].x = x;
    m->bullets[i].y = y;
    m->bullets[i].obj.pos = VEC2(x, y);
    m->bullets[i].obj.vel = VEC2(0, -1);
    broadcast_bullet_chunk(m, SPAWN, i);
}

void fire_bullet(struct moag *m, char type, float x, float y, float vx, float vy)
{
    int i = 0;
    while (m->bullets[i].active)
        if (++i >= MAX_BULLETS)
            return;
    m->bullets[i].active = 4;
    m->bullets[i].type = type;
    m->bullets[i].obj.pos = VEC2(x, y);
    m->bullets[i].x = (int)m->bullets[i].obj.pos.x;
    m->bullets[i].y = (int)m->bullets[i].obj.pos.y;
    m->bullets[i].obj.vel = VEC2(vx, vy);
    broadcast_bullet_chunk(m, SPAWN, i);
}

void fire_bullet_ang(struct moag *m, char type, int x, int y, float angle, float vel)
{
    fire_bullet(m, type, (float)x + 5.0 * cosf(DEG2RAD(angle)),
                         (float)y - 5.0 * sinf(DEG2RAD(angle)),
                         vel * cosf(DEG2RAD(angle)),
                        -vel * sinf(DEG2RAD(angle)));
}

void liquid(struct moag *m, int x, int y, int n)
{
    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x >= LAND_WIDTH)
        x = LAND_WIDTH - 1;
    if (y >= LAND_HEIGHT)
        y = LAND_HEIGHT - 1;

    int minx = LAND_WIDTH;
    int miny = LAND_HEIGHT;
    int maxx = 0;
    int maxy = 0;
    for (int i = 0; i < n; i++)
    {
        if (y < 0)
        {
            /* hit top of map, try going sideways */
            y = 0;
            for (int i = 1; i < LAND_WIDTH; i++)
            {
                if (get_land_at(m, x-i, y) == 0)
                {
                    x -= i;
                    break;
                }
                if (get_land_at(m, x+i, y) == 0)
                {
                    x += i;
                    break;
                }
            }
            set_land_at(m, x, y, 3);

            if (x < minx)
                minx = x;
            else if (x > maxx)
                maxx = x;

            if (y < miny)
                miny = y;
            else if (y > maxy)
                maxy = y;
        }

        /* nx keeps track of where to start filling next layer */
        int nx = x;
        if (get_land_at(m, x, y + 1) == 0)
        {
            y++;
        }
        else if (get_land_at(m, x - 1, y) == 0)
        {
            x--;
        }
        else if (get_land_at(m, x + 1, y) == 0)
        {
            x++;
        }
        else
        {
            /* no space below, left, right */
            /* try going right, skipping already-filled pixels */
            for (int i = x; i < LAND_WIDTH + 1; i++)
            {
                if (nx == x && get_land_at(m, i, y - 1) == 0)
                    nx = i;
                if (get_land_at(m, i, y) == 0)
                {
                    x = i;
                    break;
                }
                else if (get_land_at(m, i, y) != 3)
                {
                    /* hit a wall */
                    /* try other direction */
                    for (int i = x; i >= -1; i--)
                    {
                        if (nx == x && get_land_at(m, i, y - 1) == 0)
                            nx = i;
                        if (get_land_at(m, i, y) == 0)
                        {
                            x = i;
                            break;
                        }
                        else if (get_land_at(m, i, y) != 3)
                        {
                            /* hit a wall again, go up */
                            y--;
                            x = nx;
                            break;
                        }
                    }
                    break;
                }
            }
        }
        set_land_at(m, x, y, 3);

        if (x < minx)
            minx = x;
        else if (x > maxx)
            maxx = x;

        if (y < miny)
            miny = y;
        else if (y > maxy)
            maxy = y;
    }
    for (int iy = miny; iy <= maxy; iy++)
        for (int ix = minx; ix <= maxx; ix++)
            if (get_land_at(m, ix, iy) == 3)
                set_land_at(m, ix, iy, 1);
    broadcast_packed_land_chunk(m, minx, miny, maxx - minx + 1, maxy - miny + 1);
}

void tank_update(struct moag *m, int id)
{
    struct tank *t = &m->players[id].tank;

    if (!m->players[id].connected)
        return;

    if (m->players[id].spawn_timer > 0)
    {
        if (--m->players[id].spawn_timer <= 0)
            spawn_tank(m, id);
        return;
    }

    bool moved = false;
    bool grav = true;
    if (m->players[id].ladder_timer >= 0 && (!m->players[id].kleft || !m->players[id].kright))
        m->players[id].ladder_timer = LADDER_TIME;

    if (m->players[id].kleft && m->players[id].kright)
    {
        if (m->players[id].ladder_timer > 0)
        {
            if (get_land_at(m, t->x, t->y + 1))
                m->players[id].ladder_timer--;
            else
                m->players[id].ladder_timer = LADDER_TIME;
        }
        else if (m->players[id].ladder_timer == 0)
        {
            if (m->players[id].ladder_count > 0)
            {
                launch_ladder(m, t->x, t->y);
                m->players[id].ladder_count--;
            }
            m->players[id].ladder_timer = LADDER_TIME;
        }
    }
    else if (m->players[id].kleft)
    {
        t->facingleft = 1;
        if (get_land_at(m, t->x - 1, t->y) == 0 && t->x >= 10)
        {
            t->x--;
        }
        else if (get_land_at(m, t->x - 1, t->y - 1) == 0 && t->x >= 10)
        {
            t->x--;
            t->y--;
        }
        else if (get_land_at(m, t->x, t->y - 1) == 0 ||
                 get_land_at(m, t->x, t->y - 2) == 0 ||
                 get_land_at(m, t->x, t->y - 3) == 0)
        {
            grav = false;
            t->y--;
        }
        else
        {
            grav = false;
        }
        moved = true;
    }
    else if (m->players[id].kright)
    {
        t->facingleft = 0;
        if (get_land_at(m, t->x + 1, t->y) == 0 && t->x < LAND_WIDTH - 10)
        {
            t->x++;
        }
        else if (get_land_at(m, t->x + 1, t->y - 1) == 0 &&
                 t->x < LAND_WIDTH - 10)
        {
            t->x++;
            t->y--;
        }
        else if (get_land_at(m, t->x, t->y - 1) == 0 ||
                 get_land_at(m, t->x, t->y - 2) == 0 ||
                 get_land_at(m, t->x, t->y - 3) == 0)
        {
            grav = false;
            t->y--;
        }
        else
        {
            grav = false;
        }
        moved = true;
    }

    // Physics
    if (t->y < 20)
    {
        t->y = 20;
        moved = true;
    }

    if (grav)
    {
        if (get_land_at(m, t->x, t->y + 1) == 0)
        {
            t->y++;
            moved = true;
        }
        if (get_land_at(m, t->x, t->y + 1) == 0)
        {
            t->y++;
            moved = true;
        }
    }

    if (abs(t->x - m->crate.x) < 14 && abs(t->y - m->crate.y) < 14)
    {
        m->players[id].ladder_timer = LADDER_TIME;
        if (m->crate.type == TRIPLER)
            t->num_burst *= 3;
        else
            t->bullet = m->crate.type;
        m->crate.active = false;
        broadcast_crate_chunk(m, KILL);
        char notice[64] = "* ";
        strcat(notice, m->players[id].name);
        strcat(notice, " got ");
        switch (m->crate.type)
        {
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
            default: strcat(notice, "???"); ERR("BTYPE: %d\n", m->crate.type); break;
        }
        broadcast_chat(-1, SERVER_NOTICE, notice, strlen(notice) + 1);
    }

    // Aim
    if (m->players[id].kup && t->angle < 90)
    {
        t->angle++;
        moved = true;
    }
    else if (m->players[id].kdown && t->angle > 1)
    {
        t->angle--;
        moved = true;
    }

    // Fire
    if (t->power)
    {
        float burst_spread = 4.0;
        if (t->bullet == SHOTGUN)
        {
            t->num_burst *= SHOTGUN_PELLETS;
            burst_spread = 2.0;
        }
        int num_burst = t->bullet == MISSILE ? 1 : t->num_burst;
        float start_angle = t->facingleft ? 180 - t->angle : t->angle;
        start_angle -= (num_burst-1)*burst_spread/2.0;
        for (int i = 0; i < num_burst; i++)
        {
            fire_bullet_ang(m, t->bullet, t->x, t->y - 7,
                            start_angle + i*burst_spread,
                            (float)t->power * 0.01);
        }
        if (t->bullet != MISSILE)
            t->num_burst = 1;
        t->bullet = MISSILE;
        t->power = 0;
    }

    if (moved)
        broadcast_tank_chunk(m, MOVE, id);
}

void bounce_bullet(struct moag *m, int id, float hitx, float hity)
{
    struct bullet *b = &m->bullets[id];
    const int ix = (int)hitx;
    const int iy = (int)hity;

    if (get_land_at(m, ix, iy) == -1)
    {
        b->obj.vel = VEC2_MUL_CONST(b->obj.vel, -1);
        return;
    }

    b->obj.pos = VEC2(hitx, hity);
    b->x = ix;
    b->y = iy;

    unsigned char hit = 0;
    if (get_land_at(m, ix - 1, iy - 1)) hit |= 1 << 7;
    if (get_land_at(m, ix    , iy - 1)) hit |= 1 << 6;
    if (get_land_at(m, ix + 1, iy - 1)) hit |= 1 << 5;
    if (get_land_at(m, ix - 1, iy    )) hit |= 1 << 4;
    if (get_land_at(m, ix + 1, iy    )) hit |= 1 << 3;
    if (get_land_at(m, ix - 1, iy + 1)) hit |= 1 << 2;
    if (get_land_at(m, ix    , iy + 1)) hit |= 1 << 1;
    if (get_land_at(m, ix + 1, iy + 1)) hit |= 1;

    const float IRT2 = 0.70710678;
    const float vx = b->obj.vel.x;
    const float vy = b->obj.vel.y;

    switch (hit)
    {
        case 0x00: break;

        case 0x07: case 0xe0: case 0x02: case 0x40:
            b->obj.vel.y = -vy;
            break;

        case 0x94: case 0x29: case 0x10: case 0x08:
            b->obj.vel.x = -vx;
            break;

        case 0x16: case 0x68: case 0x04: case 0x20:
            b->obj.vel.y = vx;
            b->obj.vel.x = vy;
            break;

        case 0xd0: case 0x0b: case 0x80: case 0x01:
            b->obj.vel.y = -vx;
            b->obj.vel.x = -vy;
            break;

        case 0x17: case 0xe8: case 0x06: case 0x60:
            b->obj.vel.x = +vx * IRT2 + vy * IRT2;
            b->obj.vel.y = -vy * IRT2 + vx * IRT2;
            break;

        case 0x96: case 0x69: case 0x14: case 0x28:
            b->obj.vel.x = -vx * IRT2 + vy * IRT2;
            b->obj.vel.y = +vy * IRT2 + vx * IRT2;
            break;

        case 0xf0: case 0x0f: case 0xc0: case 0x03:
            b->obj.vel.x = +vx * IRT2 - vy * IRT2;
            b->obj.vel.y = -vy * IRT2 - vx * IRT2;
            break;

        case 0xd4: case 0x2b: case 0x90: case 0x09:
            b->obj.vel.x = -vx * IRT2 - vy * IRT2;
            b->obj.vel.y = +vy * IRT2 - vx * IRT2;
            break;

        default:
            b->obj.vel.x = -vx;
            b->obj.vel.y = -vy;
            break;
    }
}

void bullet_detonate(struct moag *m, int id)
{
    struct bullet *b = &m->bullets[id];
    float d = VEC2_MAG(b->obj.vel);

    if (d < 0.001 && d >- 0.001)
        d = d < 0 ? -1 : 1;

    const float dx = b->obj.vel.x / d;
    const float dy = b->obj.vel.y / d;

    float hitx = b->obj.pos.x;
    float hity = b->obj.pos.y;

    for (int i = 40; i > 0 && get_land_at(m, (int)hitx, (int)hity); i--)
    {
        hitx -= dx;
        hity -= dy;
    }

    switch (b->type)
    {
        case MISSILE:
            explode(m, b->x, b->y, 12, E_EXPLODE);
            break;

        case SHOTGUN:
            explode(m, b->x, b->y, 6, E_EXPLODE);
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
            explode(m, b->x, b->y, 300, E_DIRT);
            break;

        case COLLAPSE:
            explode(m, b->x, b->y, 120, E_COLLAPSE);
            break;

        case LIQUID_DIRT:
            for (int i = 0; i < 4; i++)
                set_timer(m, m->frame + 65*i, LIQUID_DIRT_WARHEAD,
                        b->x, b->y, 0, 0);
            break;

        case LIQUID_DIRT_WARHEAD:
            liquid(m, (int)hitx, (int)hity, 2000);
            break;

        case BOUNCER:
            if (b->active > 0)
                b->active = -BOUNCER_BOUNCES;
            b->active++;
            bounce_bullet(m, id, hitx, hity);
            b->obj.vel = VEC2_MUL_CONST(b->obj.vel, 0.9);
            explode(m, b->x, b->y, 12, E_EXPLODE);
            break;

        case TUNNELER:
            if (b->active > 0)
                b->active = -TUNNELER_TUNNELINGS;
            b->active++;
            explode(m, hitx, hity, 9, E_EXPLODE);
            explode(m, hitx + 8 * dx, hity + 8 * dy, 9, E_EXPLODE);
            break;

        case LADDER: {
            int x = b->x;
            int y = b->y;
            for (; y < LAND_HEIGHT; y++)
                if (get_land_at(m, x, y) == 0)
                    break;
            for (; y < LAND_HEIGHT; y++)
                if (get_land_at(m, x, y))
                    break;
            const int maxy = y + 1;
            y = b->y;
            for (; y > 0; y--)
                if (get_land_at(m, x, y) == 0)
                    break;
            const int miny = y;
            for(; y < maxy; y += 2)
            {
                set_land_at(m, x - 1, y,     0);
                set_land_at(m, x    , y,     1);
                set_land_at(m, x + 1, y,     0);
                set_land_at(m, x - 1, y + 1, 1);
                set_land_at(m, x    , y + 1, 1);
                set_land_at(m, x + 1, y + 1, 1);
            }
            broadcast_packed_land_chunk(m, x - 1, miny, 3, maxy - miny + 1);
            break;
        }

        case MIRV:
            bounce_bullet(m, id, hitx, hity);
            explode(m, b->x, b->y, 12, E_EXPLODE);
            for (int i = -3; i < 4; i++)
                fire_bullet(m, MIRV_WARHEAD,
                            b->x, b->y,
                            b->obj.vel.x + i, b->obj.vel.y);
            break;

        case MIRV_WARHEAD:
            explode(m, b->x, b->y, 30, E_EXPLODE);
            break;

        case CLUSTER_BOMB:
            bounce_bullet(m, id, hitx, hity);
            explode(m, b->x, b->y, 20, E_EXPLODE);
            for (int i = 0; i < 11; i++)
                fire_bullet(m, MISSILE, hitx, hity,
                            2.0 * cosf(i * M_PI / 5.5) + 0.50 * b->obj.vel.x,
                            2.0 * sinf(i * M_PI / 5.5) + 0.50 * b->obj.vel.y);
            break;

        case CLUSTER_BOUNCER:
            bounce_bullet(m, id, hitx, hity);
            explode(m, b->x, b->y, 20, E_EXPLODE);
            for (int i = 0; i < 11; i++)
                fire_bullet(m, BOUNCER, hitx, hity,
                            2.0 * cosf(i * M_PI / 5.5) + 0.50 * b->obj.vel.x,
                            2.0 * sinf(i * M_PI / 5.5) + 0.50 * b->obj.vel.y);
            break;

        default: break;
    }

    if (b->active >= 0)
    {
        b->active = 0;
        broadcast_bullet_chunk(m, KILL, id);
    }
}

void bullet_update(struct moag *m, int id)
{
    struct bullet *b = &m->bullets[id];

    if (!b->active)
        return;

    if (b->type == LADDER)
    {
        b->active--;
        b->obj.pos = VEC2_ADD(b->obj.pos, b->obj.vel);
        b->x = (int)b->obj.pos.x;
        b->y = (int)b->obj.pos.y;

        if (get_land_at(m, b->x, b->y) == 1)
        {
            explode(m, b->x, b->y + LADDER_LENGTH - b->active, 1, E_SAFE_EXPLODE);
            bullet_detonate(m, id);
        }
        return;
    }

    b->obj.pos = VEC2_ADD(b->obj.pos, b->obj.vel);
    b->obj.vel = VEC2_ADD(b->obj.vel, VEC2(0, GRAVITY));
    b->x = (int)b->obj.pos.x;
    b->y = (int)b->obj.pos.y;
    if (get_land_at(m, b->x, b->y))
    {
        bullet_detonate(m, id);
        return;
    }

    if (b->active > 1)
    {
        b->active--;
        return;
    }

    if (b->type == BOUNCER && b->active == 1)
        b->active = -BOUNCER_BOUNCES;
    if (b->type == TUNNELER && b->active == 1)
        b->active = -TUNNELER_TUNNELINGS;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if(DIST(m->players[i].tank.x, m->players[i].tank.y - 3,
                b->x, b->y) < 8.5)
        {
            bullet_detonate(m, id);
            return;
        }
    }

    if (m->crate.active && DIST(m->crate.x, m->crate.y - 4, b->x, b->y) < 5.5)
    {
        if (m->crate.type == TRIPLER) {
            float angle = -RAD2DEG(atan2(b->obj.vel.y, b->obj.vel.x));
            float speed = VEC2_MAG(b->obj.vel);
            fire_bullet_ang(m, b->type, b->x, b->y, angle - 20.0, speed);
            fire_bullet_ang(m, b->type, b->x, b->y, angle + 20.0, speed);
        } else if (m->crate.type == SHOTGUN) {
            bullet_detonate(m, id);
            float angle = -RAD2DEG(atan2(b->obj.vel.y, b->obj.vel.x));
            float speed = VEC2_MAG(b->obj.vel);
            int shots = SHOTGUN_PELLETS;
            for (int i = 0; i < shots; i++)
                fire_bullet_ang(m, m->crate.type, m->crate.x, m->crate.y - 4,
                        angle - (shots-1)*2 + i*4, speed*0.5);
        } else {
            bullet_detonate(m, id);
            fire_bullet(m, m->crate.type, m->crate.x, m->crate.y - 4,
                           m->crate.type != BOUNCER ? 0 :
                           b->obj.vel.x < 0 ? -0.2 :
                                               0.2, -0.2);
        }
        m->crate.active = false;
        return;
    }

    if (b->type == MIRV && b->obj.vel.y > 0)
    {
        bullet_detonate(m, id);
        return;
    }

    if (b->active)
        broadcast_bullet_chunk(m, MOVE, id);
}

void crate_update(struct moag *m)
{
    if (!m->crate.active)
    {
        m->crate.active = true;
        m->crate.x = rng_range(&m->rng, 20, LAND_WIDTH - 20);
        m->crate.y = 30;
        explode(m, m->crate.x, m->crate.y - 12, 12, E_SAFE_EXPLODE);

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
        int r = rng_range(&m->rng, 0, TOTAL);
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

    if (get_land_at(m, m->crate.x, m->crate.y + 1) == 0)
    {
        m->crate.y++;
        broadcast_crate_chunk(m, MOVE);
    }
}

void timer_update(struct moag *m, int id)
{
    struct timer *t = &m->timers[id];
    if (t->frame && t->frame <= m->frame)
    {
        fire_bullet(m, t->type, t->x, t->y, t->vx, t->vy);
        t->frame = 0;
    }
}

void step_game(struct moag *m)
{
    crate_update(m);
    for (int i = 0; i < MAX_PLAYERS; i++)
        tank_update(m, i);
    for (int i = 0; i < MAX_BULLETS; i++)
        bullet_update(m, i);
    for (int i = 0; i < MAX_TIMERS; i++)
        timer_update(m, i);
    m->frame += 1;
}

intptr_t client_connect(struct moag *m)
{
    intptr_t i = 0;
    while (m->players[i].connected)
    {
        if(++i >= MAX_PLAYERS)
        {
            printf("Client failed to connect, too many clients.\n");
            return -1;
        }
    }

    spawn_client(m, i);

    return i;
}

void handle_msg(struct moag *m, int id, const char* msg, int len)
{
    if (msg[0] == '/' && msg[1] == 'n' && msg[2] == ' ')
    {
        char notice[64] = "  ";
        strcat(notice, m->players[id].name);
        len -= 3;
        if (len > 15)
            len = 15;
        for (int i = 0; i < len; i++)
            m->players[id].name[i] = msg[i + 3];
        m->players[id].name[len] = '\0';
        strcat(notice, " is now known as ");
        strcat(notice, m->players[id].name);
        broadcast_chat(id, NAME_CHANGE, m->players[id].name, strlen(m->players[id].name) + 1);
        broadcast_chat(-1, SERVER_NOTICE, notice, strlen(notice) + 1);
    }
    else
    {
        broadcast_chat(id, CHAT, msg, len + 1);
    }
}

void init_game(struct moag *m)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
        m->players[i].connected = 0;
    for (int i = 0; i < MAX_BULLETS; i++)
        m->bullets[i].active = 0;
    for (int i = 0; i < MAX_TIMERS; i++)
        m->timers[i].frame = 0;
    m->crate.active = false;
    m->frame = 1;

    rng_seed(&m->rng, time(NULL));

    for (int y = 0; y < LAND_HEIGHT; ++y)
    {
        for (int x = 0; x < LAND_WIDTH; ++x)
        {
            if (y < LAND_HEIGHT / 3)
                set_land_at(m, x, y, 0);
            else
                set_land_at(m, x, y, 1);
        }
    }
}

void on_receive(struct moag *m, ENetEvent *ev)
{
    struct chunk_header *chunk;

    chunk = receive_chunk(ev->packet);
    intptr_t id = (intptr_t)ev->peer->data;

    switch (chunk->type)
    {
        case INPUT_CHUNK:
        {
            struct input_chunk *input = (void *)chunk;
            switch (input->key)
            {
                case KLEFT_PRESSED:   m->players[id].kleft = true; break;
                case KLEFT_RELEASED:  m->players[id].kleft = false; break;
                case KRIGHT_PRESSED:  m->players[id].kright = true; break;
                case KRIGHT_RELEASED: m->players[id].kright = false; break;
                case KUP_PRESSED:     m->players[id].kup = true; break;
                case KUP_RELEASED:    m->players[id].kup = false; break;
                case KDOWN_PRESSED:   m->players[id].kdown = true; break;
                case KDOWN_RELEASED:  m->players[id].kdown = false; break;
                case KFIRE_PRESSED:   m->players[id].kfire = true; break;
                case KFIRE_RELEASED:
                {
                    uint16_t power = input->ms;
                    m->players[id].kfire = false;
                    m->players[id].tank.power = power / 2;
                    m->players[id].tank.power = CLAMP(0, 1000, m->players[id].tank.power);
                    break;
                }
            }
            break;
        }

        case CLIENT_MSG_CHUNK:
        {
            struct client_msg_chunk *client_msg = (void *)chunk;
            handle_msg(m, id, (char *)client_msg->data, ev->packet->dataLength - 1);
            break;
        }

        default: break;
    }

    free(chunk);
}

int main(int argc, char *argv[])
{
    init_enet_server(PORT);

    LOG("Started server.\n");

    struct moag moag;
    init_game(&moag);

    LOG("Initialized game.\n");

    ENetEvent event;

    for (;;)
    {
        while (enet_host_service(get_server_host(), &event, 0))
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    LOG("Client connected.\n");
                    event.peer->data = (void *)client_connect(&moag);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    LOG("Client disconnected.\n");
                    disconnect_client(&moag, (intptr_t)event.peer->data);
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    on_receive(&moag, &event);
                    enet_packet_destroy(event.packet);
                    break;

                default:
                    break;
            }
        }

        step_game(&moag);
    }

    uninit_enet();

    LOG("Stopped server.\n");

    return EXIT_SUCCESS;
}
