
#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "enet_aux.h"

/******************************************************************************\
\******************************************************************************/

#if VERBOSITY == 5
#   define LOG(...) \
    do { fprintf(stdout, "    "); fprintf(stdout, __VA_ARGS__); } while (0)
#   define INFO(...) \
    do { fprintf(stdout, "::: "); fprintf(stdout, __VA_ARGS__); } while (0)
#   define WARN(...) \
    do { fprintf(stderr, "!!! "); fprintf(stderr, __VA_ARGS__); } while (0)
#   define ERR(...) \
    do { fprintf(stderr, "XXX "); fprintf(stderr, __VA_ARGS__); } while (0)
#elif VERBOSITY == 4
#   define LOG(...)
#   define INFO(...) \
    do { fprintf(stdout, "::: "); fprintf(stdout, __VA_ARGS__); } while (0)
#   define WARN(...) \
    do { fprintf(stderr, "!!! "); fprintf(stderr, __VA_ARGS__); } while (0)
#   define ERR(...) \
    do { fprintf(stderr, "XXX "); fprintf(stderr, __VA_ARGS__); } while (0)
#elif VERBOSITY == 3
#   define LOG(...)
#   define INFO(...)
#   define WARN(...) \
    do { fprintf(stderr, "!!! "); fprintf(stderr, __VA_ARGS__); } while (0)
#   define ERR(...) \
    do { fprintf(stderr, "XXX "); fprintf(stderr, __VA_ARGS__); } while (0)
#elif VERBOSITY == 2
#   define LOG(...)
#   define INFO(...)
#   define WARN(...)
#   define ERR(...) \
    do { fprintf(stderr, "XXX "); fprintf(stderr, __VA_ARGS__); } while (0)
#elif VERBOSITY == 1
#   define LOG(...)
#   define INFO(...)
#   define WARN(...)
#   define ERR(...)
#else
#   define LOG(...) \
    do { fprintf(stdout, "    "); fprintf(stdout, __VA_ARGS__); } while (0)
#   define INFO(...) \
    do { fprintf(stdout, "::: "); fprintf(stdout, __VA_ARGS__); } while (0)
#   define WARN(...) \
    do { fprintf(stderr, "!!! "); fprintf(stderr, __VA_ARGS__); } while (0)
#   define ERR(...) \
    do { fprintf(stderr, "XXX "); fprintf(stderr, __VA_ARGS__); } while (0)
#endif

#define DIE(...) do { ERR(__VA_ARGS__); exit(EXIT_FAILURE); } while (0)

/******************************************************************************\
\******************************************************************************/

#ifndef M_PI
#   define M_PI         3.14159
#endif

#define SQ(x)           ((x) * (x))

#define DIST(x1, y1, x2, y2) sqrt(SQ(x1 - x2) + SQ(y1 - y2))

#define DEG2RAD(deg)    ((deg) * (M_PI / 180))
#define RAD2DEG(rad)    ((rad) * (180 / M_PI))

struct vec2
{
    double x;
    double y;
};

#define VEC2(x, y) ((struct vec2){x, y})
#define VEC2_MAG(a) sqrt((a).x * (a).x + (a).y * (a).y)
#define VEC2_UNIT(a) VEC2((a).x / VEC2_MAG(a), (a).y / VEC2_MAG(a))
#define VEC2_ADD(a, b) VEC2((a).x + (b).x, (a).y + (b).y)
#define VEC2_ADD_CONST(a, f) VEC2((a).x + f, (a).y + f)
#define VEC2_SUB(a, b) VEC2((a).x - (b).x, (a).y - (b).y)
#define VEC2_SUB_CONST(a, f) VEC2((a).x - f, (a).y - f)
#define VEC2_MUL(a, b) VEC2((a).x * (b).x, (a).y * (b).y)
#define VEC2_MUL_CONST(a, f) VEC2((a).x * f, (a).y * f)
#define VEC2_DIV(a, b) VEC2((a).x / (b).x, (a).y / (b).y)
#define VEC2_DIV_CONST(a, f) VEC2((a).x / f, (a).y / f)
#define VEC2_DOT(a, b) ((a).x * (b).x + (a).y * (b).y)

/* Inclusive line intersection. Includes points. (0,0,0,0) intersects with
 * (0,0,0,0) at (0,0).
 */
static inline bool line_intersection
    (struct vec2 s1, struct vec2 e1
    ,struct vec2 s2, struct vec2 e2
    ,struct vec2 *out)
{
    struct vec2 e = VEC2_SUB(e1, s1);
    struct vec2 f = VEC2_SUB(e2, s2);
    struct vec2 p = VEC2(-e.y, e.x);

    double fp = VEC2_DOT(f, p);
    if (fp == 0)
    {
        if (s1.x <= e2.x || s1.y <= e2.y)
        {
            *out = VEC2(s1.x, s1.y);
            return true;
        }
        else if (s2.x <= e1.x || s2.y <= e1.y)
        {
            *out = VEC2(s2.x, s2.y);
            return true;
        }
        else
        {
            return false;
        }
    }

    double h = VEC2_DOT(VEC2_SUB(s1, s2), p) / fp;
    if (h >= 0.0 || h <= 1.0)
    {
        *out = VEC2_ADD(s2, VEC2_MUL_CONST(f, h));
        return true;
    }

    return false;
}

/******************************************************************************\
\******************************************************************************/

#define PORT            8080

#define MAX_PLAYERS     MAX_CLIENTS
#define MAX_BULLETS     64
#define MAX_NAME_LEN    16

#define LAND_WIDTH      800
#define LAND_HEIGHT     600

/* Chunk types. */
enum
{
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
     * 1: SERVER_MSG_CHUNK
     * 1: id
     * 1: CHAT/NAME_CHANGE/SERVER_NOTICE
     * 1: length
     * length: characters
     */
    SERVER_MSG_CHUNK,
};

enum
{
    /* RELIABLE */
    SPAWN,
    /* RELIABLE */
    KILL,
    /* UNRELIABLE */
    MOVE,
};

/* SERVER_MSG_CHUNK commands */
enum
{
    CHAT,
    SERVER_NOTICE,
    NAME_CHANGE,
};

/* WIP. Object is effected by physics. */
struct object
{
    struct vec2 pos;
    struct vec2 vel;
};
#define OBJECT(derived) ((struct object)derived)

struct tank
{
    int x, y;
    int angle, power;
    char bullet;
    bool facingleft;

    bool active;
    char name[16];
    int spawntimer;
    int ladder;
    bool kleft, kright, kup, kdown, kfire;
};

struct bullet
{
    struct object obj;
    int x, y;
    char active;
    char type;
};

struct crate
{
    int x, y;
    bool active;
    char type;
};

struct player
{
    struct tank tank;

    char name[MAX_NAME_LEN];
    bool connected;
    unsigned spawntimer;
    unsigned num_ladders;
    bool kleft, kright, kup, kdown, kfire;
};

/* Fast speed. Low memory. Period = 2^128 - 1. */
enum {XOR128_K = 4};
struct rng_state
{
    uint32_t q[XOR128_K];
};

struct moag
{
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
    for (i = 0; i < XOR128_K; ++i)
        st->q[i] = rand();
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

static inline char get_land_at(struct moag *m, int x, int y)
{
    if (x < 0 || x >= LAND_WIDTH || y < 0 || y >= LAND_HEIGHT)
        return -1;
    return m->land[y * LAND_WIDTH + x];
}

static inline void set_land_at(struct moag *m, int x, int y, char to)
{
    if (x < 0 || x >= LAND_WIDTH || y < 0 || y >= LAND_HEIGHT)
        return;
    m->land[y * LAND_WIDTH + x] = to;
}

int zip(char *in_data, size_t in_size, char **out_data, size_t *out_size);
int unzip(char *in_data, size_t in_size, char **out_data, size_t *out_size);

#endif
