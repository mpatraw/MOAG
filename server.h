
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

    size_t pos = 0;

    unsigned char *land_buffer = malloc(w * h);
    if (!land_buffer)
        goto cleanup;

    for (int yy = y; yy < h + y; ++yy)
        for (int xx = x; xx < w + x; ++xx)
            write8(land_buffer, &pos, get_land_at(m, xx, yy));

    unsigned char *zipped = NULL;
    size_t zipped_len = 0, zipped_pos = 0;
    zip((char *)land_buffer, pos, (char **)&zipped, &zipped_len);

    unsigned char *buffer = malloc(1 + sizeof(uint16_t) * 4 + zipped_len);
    if (!buffer)
        goto cleanup;

    pos = 0;
    write8(buffer, &pos, LAND_CHUNK);
    write16(buffer, &pos, x);
    write16(buffer, &pos, y);
    write16(buffer, &pos, w);
    write16(buffer, &pos, h);
    for (size_t i = 0; i < zipped_len; ++i)
        write8(buffer, &pos, read8(zipped, &zipped_pos));

    broadcast_packet(buffer, pos, true);
    INFO("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, zipped_len);

cleanup:
    if (land_buffer)
        free(land_buffer);
    if (zipped)
        free(zipped);
    if (buffer)
        free(buffer);
}

static inline void broadcast_tank_chunk(struct moag *m, int type, int id)
{
    unsigned char buffer[TANK_CHUNK_SIZE];
    size_t pos = 0;

    write8(buffer, &pos, TANK_CHUNK);
    write8(buffer, &pos, type);
    write8(buffer, &pos, id);

    if (type != KILL)
    {
        write16(buffer, &pos, m->tanks[id].x);
        write16(buffer, &pos, m->tanks[id].y);

        if (m->tanks[id].facingleft)
            write8(buffer, &pos, -m->tanks[id].angle);
        else
            write8(buffer, &pos, m->tanks[id].angle);
    }

    if (type == SPAWN || type == KILL)
        broadcast_packet(buffer, pos, true);
    else
        broadcast_packet(buffer, pos, false);
    INFO("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, pos);
}

static inline void broadcast_bullet_chunk(struct moag *m, int type, int id)
{
    unsigned char buffer[BULLET_CHUNK_SIZE];
    size_t pos = 0;

    write8(buffer, &pos, BULLET_CHUNK);
    write8(buffer, &pos, type);
    write8(buffer, &pos, id);

    if (type != KILL)
    {
        write16(buffer, &pos, m->bullets[id].x);
        write16(buffer, &pos, m->bullets[id].y);
    }

    if (type == SPAWN || type == KILL)
        broadcast_packet(buffer, pos, true);
    else
        broadcast_packet(buffer, pos, false);
    INFO("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, pos);
}

static inline void broadcast_crate_chunk(struct moag *m, int type)
{
    unsigned char buffer[CRATE_CHUNK_SIZE];
    size_t pos = 0;

    write8(buffer, &pos, CRATE_CHUNK);
    write8(buffer, &pos, type);

    if (type != KILL)
    {
        write16(buffer, &pos, m->crate.x);
        write16(buffer, &pos, m->crate.y);
    }

    if (type == SPAWN || type == KILL)
        broadcast_packet(buffer, pos, true);
    else
        broadcast_packet(buffer, pos, false);

    INFO("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, pos);
}

static inline void broadcast_chat(int id, char cmd, const char *msg, unsigned char len)
{
    unsigned char buffer[SERVER_MSG_CHUNK_SIZE];
    size_t pos = 0;

    write8(buffer, &pos, SERVER_MSG_CHUNK);
    write8(buffer, &pos, id);
    write8(buffer, &pos, cmd);
    write8(buffer, &pos, len);
    for(int i=0;i<len;i++)
        write8(buffer, &pos, msg[i]);

    broadcast_packet(buffer, pos, true);
    INFO("%u: %s: %zu\n", (unsigned)time(NULL), __PRETTY_FUNCTION__, pos);
}


#endif
