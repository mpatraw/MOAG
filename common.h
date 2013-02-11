
#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <enet/enet.h>

/******************************************************************************\
General macros.
\******************************************************************************/

#if VERBOSE
#   define LOG(...) \
    do { fprintf(stdout, "= "); fprintf(stdout, __VA_ARGS__); } while (0)
#   define ERR(...) \
    do { fprintf(stderr, "! "); fprintf(stderr, __VA_ARGS__); } while (0)
#else
#   define LOG(...)
#   define ERR(...) \
    do { fprintf(stderr, "! "); fprintf(stderr, __VA_ARGS__); } while (0)
#endif
#define DIE(...) \
    do { ERR(__VA_ARGS__); exit(EXIT_FAILURE); } while (0)

#define SQ(x)           ((x) * (x))

#ifndef M_PI
#   define M_PI         3.14159
#endif

#define DEG2RAD(deg)    ((deg) * (M_PI / 180))
#define RAD2DEG(rad)    ((rad) * (180 / M_PI))

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define CLAMP(min, max, val) MAX(min, MIN(max, val))
#define WITHIN(min, max, val) ((val) >= (min) && (val) <= (max))
#define LERP(a, b, t) ((a) + (t) * ((b) - (a))

#define DIST(x1, y1, x2, y2) sqrt(SQ(x1 - x2) + SQ(y1 - y2))

/******************************************************************************\
Core utility functions.
\******************************************************************************/

void safe_malloc_set_callback(void (*callback) (int, size_t));
void *safe_malloc(size_t len);
void *safe_realloc(void *mem, size_t len);
char *string_duplicate(const char *str);

/******************************************************************************\
Random number generator.
\******************************************************************************/

/* Fast speed. Low memory. Period = 2^128 - 1. */
enum {XOR128_K = 4};
struct rng_state
{
    uint32_t q[XOR128_K];
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

/******************************************************************************\
Networking.
\******************************************************************************/

#define PORT            8080

#define MAX_PLAYERS     MAX_CLIENTS
#define CONNECT_TIMEOUT 10000
#define MAX_CLIENTS     8
#define NUM_CHANNELS    2

void init_enet_client(const char *ip, unsigned port);
void init_enet_server(unsigned port);
void uninit_enet(void);

ENetHost *get_client_host(void);
ENetHost *get_server_host(void);
ENetPeer *get_peer(void);

static inline void send_packet(uint8_t *buf, size_t len, bool broadcast, bool reliable)
{
    uint32_t flags = 0;
    if (reliable)
        flags |= ENET_PACKET_FLAG_RELIABLE;
    ENetPacket *packet = enet_packet_create(NULL, len, flags);
    memcpy(packet->data, buf, len);
    if (broadcast)
        enet_host_broadcast(get_server_host(), 0, packet);
    else
        enet_peer_send(get_peer(), 1, packet);
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

/* RLE */
uint8_t* rlencode(uint8_t *src, const size_t len, size_t* outlen);
uint8_t* rldecode(uint8_t *src, const size_t len, size_t* outlen);

/******************************************************************************\
Physics.
\******************************************************************************/

#define EPSILON         0.000001

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

static inline bool vec_inbetween_line(struct line l, struct vec2 v)
{
    return fabs(VEC2_DIST(l.beg, v) +
                VEC2_DIST(l.end, v) -
                VEC2_DIST(l.beg, l.end)) < EPSILON;
}

/* Inclusive line intersection. Includes points. (0,0,0,0) intersects with
 * (0,0,0,0) at (0,0).
 */
static inline bool lines_intersection
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
        if (vec_inbetween_line(first, second.beg))
        {
            if (out)
                *out = second.beg;
            return true;
        }
        else if (vec_inbetween_line(first, second.end))
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

static inline bool rects_intersecting(struct rect r1, struct rect r2)
{
    bool xoverlap = WITHIN(r1.tl.x, r1.br.x, RECT_X(r2)) ||
                    WITHIN(r2.tl.x, r2.br.x, RECT_X(r1));

    bool yoverlap = WITHIN(r1.tl.y, r1.br.y, RECT_Y(r2)) ||
                    WITHIN(r2.tl.y, r2.br.y, RECT_Y(r1));

    return xoverlap && yoverlap;
}

static inline bool rect_intersecting_line(struct rect r, struct line l)
{
    return lines_intersection(l, RECT_LEFT_LINE(r), NULL) ||
           lines_intersection(l, RECT_RIGHT_LINE(r), NULL) ||
           lines_intersection(l, RECT_TOP_LINE(r), NULL) ||
           lines_intersection(l, RECT_BOTTOM_LINE(r), NULL);
}

static inline bool vec_within_rect(struct rect r, struct vec2 v)
{
    return WITHIN(r.tl.x, r.br.x, v.x) &&
           WITHIN(r.tl.y, r.br.y, v.y);
}

static inline bool line_within_rect(struct rect r, struct line l)
{
    return vec_within_rect(r, l.beg) &&
           vec_within_rect(r, l.end);
}

static inline bool line_overlaps_rect(struct rect r, struct line l)
{
    return vec_within_rect(r, l.beg) ||
           vec_within_rect(r, l.end);
}

/******************************************************************************\
Shared structures.
\******************************************************************************/

#define INPUT_CHUNK_SIZE        4
#define CLIENT_MSG_CHUNK_SIZE   258
#define TANK_CHUNK_SIZE         8
#define BULLET_CHUNK_SIZE       7
#define CRATE_CHUNK_SIZE        6
#define SERVER_MSG_CHUNK_SIZE   260

/* Chunk types. */
enum
{
    /******************
     * Client -> Server
     */

    /* RELIABLE
     * 1: INPUT_CHUNK
     * 1: *_*_CHUNK
     * 2: Milliseconds held.
     */
    INPUT_CHUNK,

    /* RELIABLE
     * 1: CLIENT_MSG_CHUNK
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
    /* RELIABLE
     * 1: PACKED_LAND_CHUNK
     * 2: x-position
     * 2: y-position
     * 2: width
     * 2: height
     * 4: packed-size
     * X: RLE-compressed data
     */
    PACKED_LAND_CHUNK,
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
     * length: characters
     */
    SERVER_MSG_CHUNK,
};

/* Input types.
 * KFIRE_RELEASED is special. It has extra info on the time spent charging up.
 */
enum
{
    KLEFT_PRESSED, KLEFT_RELEASED,
    KRIGHT_PRESSED, KRIGHT_RELEASED,
    KUP_PRESSED, KUP_RELEASED,
    KDOWN_PRESSED, KDOWN_RELEASED,
    KFIRE_PRESSED, KFIRE_RELEASED,
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

/* The following structs must be tightly packed
 * because they're used to read packet data. */
#ifdef _MSC_VER
#  define PACKED_STRUCT(name) \
    __pragma(pack(push, 1)) struct name __pragma(pack(pop))
#elif defined(__GNUC__)
#  define PACKED_STRUCT(name) struct __attribute__((packed)) name
#endif

PACKED_STRUCT(chunk_header)
{
    uint8_t type;
};

PACKED_STRUCT(input_chunk)
{
    struct chunk_header _;
    uint8_t key;
    uint16_t ms;
};

PACKED_STRUCT(client_msg_chunk)
{
    struct chunk_header _;
    uint8_t data[];
};

PACKED_STRUCT(land_chunk)
{
    struct chunk_header _;
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    uint8_t data[];
};

PACKED_STRUCT(packed_land_chunk)
{
    struct chunk_header _;
    int16_t x;
    int16_t y;
    int16_t width;
    int16_t height;
    uint8_t data[];
};

PACKED_STRUCT(tank_chunk)
{
    struct chunk_header _;
    uint8_t action;
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint8_t angle;
};

PACKED_STRUCT(bullet_chunk)
{
    struct chunk_header _;
    uint8_t action;
    uint8_t id;
    uint16_t x;
    uint16_t y;
};

PACKED_STRUCT(crate_chunk)
{
    struct chunk_header _;
    uint8_t action;
    uint16_t x;
    uint16_t y;
};

PACKED_STRUCT(server_msg_chunk)
{
    struct chunk_header _;
    uint8_t id;
    uint8_t action;
    uint8_t data[];
};

struct chunk_header *receive_chunk(ENetPacket *packet);
void send_chunk(struct chunk_header *chunk, size_t len, bool broadcast, bool reliable);

/******************************************************************************\
\******************************************************************************/

#define MAX_BULLETS     64
#define MAX_TIMERS      64
#define MAX_NAME_LEN    16

#define LAND_WIDTH      800
#define LAND_HEIGHT     600

/* WIP. Object is effected by physics. */
struct object
{
    struct vec2 pos;
    struct vec2 vel;
};
#define OBJECT(derived) ((struct object)derived)

struct tank
{
    struct object obj;
    int x, y;
    int angle, power;
    char bullet;
    int num_burst;
    bool facingleft;
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
    struct object obj;
    int x, y;
    bool active;
    char type;
};

struct player
{
    struct tank tank;

    char name[MAX_NAME_LEN];
    bool connected;
    unsigned spawn_timer;
    unsigned ladder_count;
    int ladder_timer;
    bool kleft, kright, kup, kdown, kfire;
};

struct timer
{
    int frame; // 0 for inactive
    char type;
    float x, y, vx, vy;
};

struct moag
{
    struct player players[MAX_PLAYERS];
    struct bullet bullets[MAX_BULLETS];
    struct timer timers[MAX_TIMERS];
    struct crate crate;
    char land[LAND_WIDTH * LAND_HEIGHT];
    struct rng_state rng;
    int frame;
};

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

#endif
