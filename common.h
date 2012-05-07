
#ifndef COMMON_H
#define COMMON_H

#define PORT            8080
#define MAX_CLIENTS     8
#define NUM_CHANNELS    2

/* 5 seconds. */

#define LAND_WIDTH      800
#define LAND_HEIGHT     600

enum {LAND_CHUNK, TANK_CHUNK, BULLET_CHUNK, MSG_CHUNK, CRATE_CHUNK};

struct tank {
    int x, y, lastx, lasty;
    int angle, power;
    int spawntimer;
    int ladder;
    char active;
    char name[16];
    char bullet;
    char facingLeft;
    char kleft, kright, kup, kdown, kfire;
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

inline char get_land_at(char *land, int x, int y)
{
    if (x < 0 || x >= LAND_WIDTH || y < 0 || y >= LAND_HEIGHT)
        return -1;
    return land[y * LAND_WIDTH + x];
}

inline void set_land_at(char *land, int x, int y, char to)
{
    if (x < 0 || x >= LAND_WIDTH || y < 0 || y >= LAND_HEIGHT)
        return;
    land[y * LAND_WIDTH + x] = to;
}

#endif
