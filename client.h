
#ifndef CLIENT_H
#define CLIENT_H

#include "common.h"
#include "sdl_aux.h"

#define BUFLEN          256
#define CHAT_LINES      7
#define CHAT_EXPIRETIME 18000

struct chatline
{
    int expire;
    char *str;
};

static inline void send_byte(unsigned char c)
{
    send_packet(&c, 1, true);
}

static inline void read_land_chunk(struct moag *m, unsigned char *packet, size_t len)
{
    /* Skip LAND_CHUNK */
    size_t pos = 1;

    int x = read16(packet, &pos);
    int y = read16(packet, &pos);
    int w = read16(packet, &pos);
    int h = read16(packet, &pos);

    if (w < 0) w = 0;
    if (h < 0) h = 0;
    if (x < 0 || y < 0 || x + w > LAND_WIDTH || y + h > LAND_HEIGHT)
        return;

    if (pos == len)
    {
        WARN("LAND_CHUNK length == 0.\n");
        return;
    }

    unsigned char *unzipped = NULL;
    size_t unzipped_len = 0;
    unzip((char *)packet + pos, len - pos, (char **)&unzipped, &unzipped_len);

    pos = 0;
    for (int yy = y; yy < h + y; ++yy)
        for (int xx = x; xx < w + x; ++xx)
            set_land_at(m, xx, yy, read8(unzipped, &pos));

    free(unzipped);
}

#endif
