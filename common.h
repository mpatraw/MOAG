
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

#define EPSILON         0.000001

#define SQ(x)           ((x) * (x))

#define DIST(x1, y1, x2, y2) sqrt(SQ(x1 - x2) + SQ(y1 - y2))

#define DEG2RAD(deg)    ((deg) * (M_PI / 180))
#define RAD2DEG(rad)    ((rad) * (180 / M_PI))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(min, max, val) MAX(min, MIN(max, val))
#define WITHIN(min, max, val) ((val) >= (min) && (val) <= (max))
#define LERP(a, b, t) ((a) + (t) * ((b) - (a))

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

#define VEC2_DIST(a, b) sqrt(SQ((a).x - (b.x)) + SQ((a).y - (b).y))

struct line
{
    struct vec2 beg;
    struct vec2 end;
};

#define LINE LINE_XYXY
#define LINE_VV(v1, v2) ((struct line){v1, v2})
#define LINE_XYXY(x1, y1, x2, y2) LINE_VV(VEC2(x1, y1), VEC2(x2, y2))

static inline bool line_vec_inbetween(struct line l, struct vec2 v)
{
    return fabs(VEC2_DIST(l.beg, v) + VEC2_DIST(l.end, v) -
                VEC2_DIST(l.beg, l.end)) < EPSILON;
}

/* Inclusive line intersection. Includes points. (0,0,0,0) intersects with
 * (0,0,0,0) at (0,0).
 */
static inline bool line_intersection
    (struct line first
    ,struct line second
    ,struct vec2 *out)
{
    struct vec2 e = VEC2_SUB(first.end, first.beg);
    struct vec2 f = VEC2_SUB(second.end, second.beg);
    struct vec2 p = VEC2(-e.y, e.x);
    struct vec2 q = VEC2(-f.y, f.x);

    double fp = VEC2_DOT(f, p);
    if (fabs(fp) < EPSILON)
    {
        if (line_vec_inbetween(first, second.beg))
        {
            if (out)
                *out = second.beg;
            return true;
        }
        else if (line_vec_inbetween(first, second.end))
        {
            if (out)
                *out = second.end;
            return true;
        }
        else
        {
            return false;
        }
    }

    double h = VEC2_DOT(VEC2_SUB(first.beg, second.beg), p) / fp;
    double g = VEC2_DOT(VEC2_SUB(first.beg, second.beg), q) / fp;
    if (h >= 0.0 && h <= 1.0 && g >= 0.0 && g <= 1.0)
    {
        if (out)
            *out = VEC2_ADD(second.beg, VEC2_MUL_CONST(f, h));
        return true;
    }

    return false;
}

struct rect
{
    struct vec2 tl;
    struct vec2 br;
};

#define RECT RECT_XYWH
#define RECT_VV(v1, v2) ((struct rect){v1, v2})
#define RECT_XYXY(x1, y1, x2, y2) RECT_VV(VEC2(x1, y1), VEC2(x2, y2))
#define RECT_XYWH(x, y, w, h) RECT_XYXY(x, y, x + w, y + w)
#define RECT_X(r) ((r).tl.x)
#define RECT_Y(r) ((r).tl.y)
#define RECT_WIDTH(r) ((r).br.x - (r).tl.x)
#define RECT_HEIGHT(r) ((r).br.y - (r).tl.y)

#define RECT_ADD(r, v) RECT_VV(VEC2_ADD((r).tl, (v)), VEC2_ADD((r).br, (v))
#define RECT_SUB(r, v) RECT_VV(VEC2_SUB((r).tl, (v)), VEC2_SUB((r).br, (v))
#define RECT_MUL(r, v) RECT_VV(VEC2_MUL((r).tl, (v)), VEC2_MUL((r).br, (v))
#define RECT_DIV(r, v) RECT_VV(VEC2_DIV((r).tl, (v)), VEC2_DIV((r).br, (v))

#define RECT_LEFT_LINE(r) LINE((r).tl.x, (r).tl.y, (r).tl.x, (r).br.y)
#define RECT_RIGHT_LINE(r) LINE((r).br.x, (r).tl.y, (r).br.x, (r).br.y)
#define RECT_TOP_LINE(r) LINE((r).tl.x, (r).tl.y, (r).br.x, (r).tl.y)
#define RECT_BOTTOM_LINE(r) LINE((r).tl.x, (r).br.y, (r).br.x, (r).br.y)

static inline bool rect_intersecting(struct rect r1, struct rect r2)
{
    bool xoverlap = WITHIN(r1.tl.x, r1.br.x, RECT_X(r2)) ||
                    WITHIN(r2.tl.x, r2.br.x, RECT_X(r1));

    bool yoverlap = WITHIN(r1.tl.y, r1.br.y, RECT_Y(r2)) ||
                    WITHIN(r2.tl.y, r2.br.y, RECT_Y(r1));

    return xoverlap && yoverlap;
}

static inline bool rect_line_intersection
    (struct rect r
    ,struct line l
    ,struct vec2 *out)
{
    return line_intersection(l, RECT_LEFT_LINE(r), out) ||
           line_intersection(l, RECT_RIGHT_LINE(r), out) ||
           line_intersection(l, RECT_TOP_LINE(r), out) ||
           line_intersection(l, RECT_BOTTOM_LINE(r), out);
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
