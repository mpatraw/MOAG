
#ifndef COMMON_H
#define COMMON_H

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "enet_aux.h"

#define M_PI            3.14159

#define PORT            8080

#define MAX_PLAYERS     MAX_CLIENTS
#define MAX_BULLETS     32
#define MAX_NAME_LEN    16

#define LAND_WIDTH      800
#define LAND_HEIGHT     600

/* Chunk types. */
enum {
    /******************
     * Client -> Server
     */

    /* RELIABLE
     * 1: *_*_CHUNK
     */
    KLEFT_PRESSED_CHUNK, KLEFT_RELEASED_CHUNK,
    KRIGHT_PRESSED_CHUNK, KRIGHT_RELEASED_CHUNK,
    KUP_PRESSED_CHUNK, KUP_RELEASED_CHUNK,
    KDOWN_PRESSED_CHUNK, KDOWN_RELEASED_CHUNK,
    KFIRE_PRESSED_CHUNK, KFIRE_RELEASED_CHUNK,

    /* RELIABLE
     * 1: CLIENT_MSG_CHUNK
     * 1: length
     * length: characters
     */
    CLIENT_MSG_CHUNK,

    /******************
     * Server -> Client
     */

    /* RELIABLE
     * 1: LAND_CHUNK
     * 2: x-position
     * 2: y-position
     * 2: width
     * 2: height
     * X: zipped land (columns first)
     */
    LAND_CHUNK,
    /* VARIES
     * 1: TANK_CHUNK
     * 1: SPAWN/KILL/MOVE
     * 1: id
     * IF NOT KILL
     *  2: x-position
     *  2: y-position
     *  1: angle (0 to 180), 0 is left, 180 is right
     */
    TANK_CHUNK,
    /* VARIES
     * 1: BULLET_CHUNK
     * 1: SPAWN/KILL/MOVE
     * 1: id
     * IF NOT KILL
     *  2: x-position
     *  2: y-position
     */
    BULLET_CHUNK,
    /* VARIES
     * 1: CREATE_CHUNK
     * 1: SPAWN/KILL/MOVE
     * IF NOT KILL
     *  2: x-position
     *  2: y-position
     */
    CRATE_CHUNK,
    /* RELIABLE
     * 1: MSG_CHUNK
     * 1: id
     * 1: CHAT/NAME_CHANGE/SERVER_NOTICE
     * 1: length
     * length: characters
     */
    MSG_CHUNK,
};

enum {
    /* RELIABLE */
    SPAWN,
    /* RELIABLE */
    KILL,
    /* UNRELIABLE */
    MOVE,
};

/* MSG_CHUNK commands */
enum {
    CHAT,
    SERVER_NOTICE,
    NAME_CHANGE,
};

struct tank {
    int x, y;
    int angle, power;
    char bullet;
    bool facingLeft;

    bool active;
    char name[16];
    int spawntimer;
    int ladder;
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

struct player {
    struct tank tank;

    char name[MAX_NAME_LEN];
    bool connected;
    unsigned spawntimer;
    unsigned num_ladders;
    bool kleft, kright, kup, kdown, kfire;
};

/* Fast speed. Low memory. Period = 2^128 - 1. */
enum {XOR128_K = 4};
struct rng_state {
    uint32_t q[XOR128_K];
};

struct moag {
    struct player players[MAX_PLAYERS];
    struct tank tanks[MAX_PLAYERS];
    struct bullet bullets[MAX_BULLETS];
    struct crate crate;
    char land[LAND_WIDTH * LAND_HEIGHT];
    struct rng_state rng;
};

static inline void rng_seed(struct rng_state *st, uint32_t seed)
{
    int i;

    srand(seed);
    for (i = 0; i < XOR128_K; ++i) {
        st->q[i] = rand();
    }
}

static inline uint32_t rng_u32(struct rng_state *st)
{
    uint32_t t;
    t = (st->q[0] ^ (st->q[0] << 11));
    st->q[0] = st->q[1];
    st->q[1] = st->q[2];
    st->q[2] = st->q[3];
    return st->q[3] = st->q[3] ^ (st->q[3] >> 19) ^ (t ^ (t >> 8));
}

static inline double rng_unit(struct rng_state *st)
{
    return rng_u32(st) * 2.3283064365386963e-10;
}

static inline double rng_under(struct rng_state *st, int32_t max)
{
    return rng_unit(st) * max;
}

static inline double rng_between(struct rng_state *st, int32_t min, int32_t max)
{
    return rng_under(st, max - min) + min;
}

static inline int32_t rng_range(struct rng_state *st, int32_t min, int32_t max)
{
    return floor(rng_between(st, min, max + 1));
}

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

int zip(char *in_data, size_t in_size, char **out_data, size_t *out_size);
int unzip(char *in_data, size_t in_size, char **out_data, size_t *out_size);

#endif
