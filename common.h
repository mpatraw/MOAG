
#ifndef COMMON_H
#define COMMON_H


#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define M_PI            3.14159

#define PORT            8080

#define MAX_BULLETS     32

#define LAND_WIDTH      320
#define LAND_HEIGHT     480

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

#endif
