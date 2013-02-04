
#ifndef SERVER_H
#define SERVER_H

#include "common.h"

#define GRAVITY             0.1
#define BOUNCER_BOUNCES     11
#define TUNNELER_TUNNELINGS 20
#define RESPAWN_TIME        40
#define LADDER_TIME         60
#define LADDER_LENGTH       64

enum
{
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
};

enum
{
    E_EXPLODE,
    E_DIRT,
    E_SAFE_EXPLODE,
    E_COLLAPSE
};

static inline void broadcast_land_chunk(struct moag *m, int x, int y, int w, int h)
{
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > LAND_WIDTH) w = LAND_WIDTH - x;
    if (y + h > LAND_HEIGHT) h = LAND_HEIGHT - y;
    if (w <= 0 || h <= 0 || x + w > LAND_WIDTH || y + h > LAND_HEIGHT)
        return;

    struct land_chunk *chunk = safe_malloc(sizeof *chunk + w * h);

    chunk->_.type = LAND_CHUNK;
    chunk->x = x;
    chunk->y = y;
    chunk->width = w;
    chunk->height = h;

    int i = 0;
    for (int yy = y; yy < h + y; ++yy)
    {
        for (int xx = x; xx < w + x; ++xx)
        {
            chunk->data[i] = get_land_at(m, xx, yy);
            i++;
        }
    }
    send_chunk((void *)chunk, sizeof *chunk + w * h, true, true);

    LOG("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, sizeof *chunk + w * h);
    free(chunk);
}

static inline void broadcast_tank_chunk(struct moag *m, int action, int id)
{
    struct tank_chunk chunk;
    chunk._.type = TANK_CHUNK;
    chunk.action = action;
    chunk.id = id;
    chunk.x = m->players[id].tank.x;
    chunk.y = m->players[id].tank.y;
    if (m->players[id].tank.facingleft)
        chunk.angle = -m->players[id].tank.angle;
    else
        chunk.angle = m->players[id].tank.angle;

    if (action == SPAWN || action == KILL)
        send_chunk((void *)&chunk, sizeof chunk, true, true);
    else
        send_chunk((void *)&chunk, sizeof chunk, true, false);

    LOG("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, sizeof chunk);
}

static inline void broadcast_bullet_chunk(struct moag *m, int action, int id)
{
    struct bullet_chunk chunk;
    chunk._.type = BULLET_CHUNK;
    chunk.action = action;
    chunk.id = id;
    chunk.x = m->bullets[id].x;
    chunk.y = m->bullets[id].y;

    if (action == SPAWN || action == KILL)
        send_chunk((void *)&chunk, sizeof chunk, true, true);
    else
        send_chunk((void *)&chunk, sizeof chunk, true, false);

    LOG("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, sizeof chunk);
}

static inline void broadcast_crate_chunk(struct moag *m, int action)
{
    struct crate_chunk chunk;
    chunk._.type = CRATE_CHUNK;
    chunk.action = action;
    chunk.x = m->crate.x;
    chunk.y = m->crate.y;

    if (action == SPAWN || action == KILL)
        send_chunk((void *)&chunk, sizeof chunk, true, true);
    else
        send_chunk((void *)&chunk, sizeof chunk, true, false);

    LOG("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, sizeof chunk);
}

static inline void broadcast_chat(int id, char action, const char *msg, unsigned char len)
{
    struct server_msg_chunk *chunk = safe_malloc(sizeof *chunk + len);

    chunk->_.type = SERVER_MSG_CHUNK;
    chunk->id = id;
    chunk->action = action;

    for (int i = 0; i < len; ++i)
        chunk->data[i] = msg[i];

    send_chunk((void *)chunk, sizeof *chunk + len, true, true);

    LOG("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, sizeof *chunk + len);
    free(chunk);
}


#endif
