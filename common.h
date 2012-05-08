
#ifndef COMMON_H
#define COMMON_H


#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <enet/enet.h>
#include <netinet/in.h>

#define M_PI            3.14159

#define PORT            8080
#define MAX_CLIENTS     8
#define NUM_CHANNELS    2

#define MAX_BULLETS     256

#define LAND_WIDTH      800
#define LAND_HEIGHT     600

enum {
    /* Client -> Server */
    KLEFT_PRESSED_CHUNK, KLEFT_RELEASED_CHUNK,
    KRIGHT_PRESSED_CHUNK, KRIGHT_RELEASED_CHUNK,
    KUP_PRESSED_CHUNK, KUP_RELEASED_CHUNK,
    KDOWN_PRESSED_CHUNK, KDOWN_RELEASED_CHUNK,
    KFIRE_PRESSED_CHUNK, KFIRE_RELEASED_CHUNK,
    /* Server -> Client */
    LAND_CHUNK, TANK_CHUNK, BULLET_CHUNK, MSG_CHUNK, CRATE_CHUNK
};

struct tank {
    int x, y;
    int angle, power;
    int spawntimer;
    int ladder;
    bool active;
    char name[16];
    char bullet;
    bool facingLeft;
    bool kleft, kright, kup, kdown, kfire;
};

struct bullet {
    int x, y;
    float fx, fy, vx, vy;
    char active;
    char type;
};

struct crate {
    int x, y;
    char type;
};

static inline char get_land_at(char *land, int x, int y)
{
    if (x < 0 || x >= LAND_WIDTH || y < 0 || y >= LAND_HEIGHT)
        return -1;
    return land[y * LAND_WIDTH + x];
}

static inline void set_land_at(char *land, int x, int y, char to)
{
    if (x < 0 || x >= LAND_WIDTH || y < 0 || y >= LAND_HEIGHT)
        return;
    land[y * LAND_WIDTH + x] = to;
}

static inline void write8(unsigned char *buf, size_t *pos, uint8_t val)
{
    *(unsigned char *)(&buf[*pos]) = val;
    (*pos) += 1;
}

static inline void write16(unsigned char *buf, size_t *pos, uint16_t val)
{
    *(uint16_t *)(&buf[*pos]) = htons(val);
    (*pos) += 2;
}

static inline void write32(unsigned char *buf, size_t *pos, uint32_t val)
{
    *(uint32_t *)(&buf[*pos]) = htonl(val);
    (*pos) += 4;
}

static inline uint8_t read8(unsigned char *buf, size_t *pos)
{
    uint8_t val = *(char *)(&buf[*pos]);
    (*pos) += 1;
    return val;
}

static inline uint16_t read16(unsigned char *buf, size_t *pos)
{
    uint16_t val = ntohs(*(uint16_t *)(&buf[*pos]));
    (*pos) += 2;
    return val;
}

static inline uint32_t read32(unsigned char *buf, size_t *pos)
{
    uint32_t val = ntohl(*(uint32_t *)(&buf[*pos]));
    (*pos) += 4;
    return val;
}

int die(const char *fmt, ...);

#endif
