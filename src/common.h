
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

#include "config.h"
#include "moag.h"

#define SQ(x)           ((x) * (x))

#ifndef M_PI
#   define M_PI         3.14159
#endif

#define DEG2RAD(deg)    ((deg) * (M_PI / 180))
#define RAD2DEG(rad)    ((rad) * (180 / M_PI))

/******************************************************************************\
Networking.
\******************************************************************************/

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

struct tank
{
    int x, y;
    int velx, vely;
    int angle, power;
    char bullet;
    int num_burst;
    bool facingleft;
};

struct bullet
{
    int x, y;
    int velx, vely;
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
    struct player players[g_max_players];
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
