
#include <errno.h>
#include <zlib.h>

#include "common.h"

static void (*safe_malloc_callback) (int error_number, size_t requested);

void safe_malloc_set_callback(void (*callback) (int, size_t))
{
    safe_malloc_callback = callback;
}

void *safe_malloc(size_t len)
{
    void *p;
    if (len == 0)
        len = 1;
    p = malloc(len);
    if (!p)
    {
        if (safe_malloc_callback)
            safe_malloc_callback(errno, len);
        exit(EXIT_FAILURE);
    }
    return p;
}

void *safe_realloc(void *mem, size_t len)
{
    void *p;
    if (len == 0)
        len = 1;
    p = realloc(mem, len);
    if (!p)
    {
        if (safe_malloc_callback)
            safe_malloc_callback(errno, len);
        exit(EXIT_FAILURE);
    }
    return p;
}

char *string_duplicate(const char *str)
{
    char *s = safe_malloc(strlen(str) + 1);
    strcpy(s, str);
    return s;
}

/******************************************************************************\
\******************************************************************************/

// convenience hack for old enet support
#if ENET_VERSION_MAJOR == 1 && ENET_VERSION_MINOR == 2
#define OLD_ENET
#endif

static bool _initialized = false;

static ENetHost *_client = NULL;
static ENetHost *_server = NULL;
static ENetPeer *_peer = NULL;

void init_enet_client(const char *ip, unsigned port)
{
    if (!_initialized)
    {
        if (enet_initialize() != 0)
            DIE("An error occurred while initializing ENet.\n");
        else
            atexit(enet_deinitialize);
    }
    _initialized = true;

#ifdef OLD_ENET
    _client = enet_host_create(NULL, MAX_CLIENTS, 0, 0);
#else
    _client = enet_host_create(NULL, MAX_CLIENTS, NUM_CHANNELS, 0, 0);
#endif
    if (!_client)
        DIE("An error occurred while trying to create an ENet client host.\n");

    ENetAddress address;
    enet_address_set_host(&address, ip);
    address.port = port;

#ifdef OLD_ENET
    _peer = enet_host_connect(_client, &address, NUM_CHANNELS);
#else
    _peer = enet_host_connect(_client, &address, NUM_CHANNELS, 0);
#endif
    if (!_peer)
        DIE("No available peers for initiating an ENet connection.\n");

    ENetEvent ev;

    if (enet_host_service(_client, &ev, CONNECT_TIMEOUT) == 0 ||
        ev.type != ENET_EVENT_TYPE_CONNECT)
    {
        enet_peer_reset(_peer);
        DIE("Connection to %s timed out.\n", ip);
    }
}

void init_enet_server(unsigned port)
{
    if (!_initialized)
    {
        if (enet_initialize() != 0)
            DIE("An error occurred while initializing ENet.\n");
        else
            atexit(enet_deinitialize);
    }
    _initialized = true;

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

#ifdef OLD_ENET
    _server = enet_host_create(&address, MAX_CLIENTS, 0, 0);
#else
    _server = enet_host_create(&address, MAX_CLIENTS, NUM_CHANNELS, 0, 0);
#endif
    if (!_server)
        DIE("An error occurred while trying to create an ENet server host.\n");
}

void uninit_enet(void)
{
    if (_peer)
    {
        enet_peer_disconnect(_peer, 0);

        ENetEvent ev;
        while (enet_host_service(_client, &ev, 3000))
        {
            switch (ev.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(ev.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                goto successfull_disconnect;

            default:
                break;
            }
        }

        enet_peer_reset(_peer);
    }

successfull_disconnect:
    if (_client)
        enet_host_destroy(_client);
    if (_server)
        enet_host_destroy(_server);

    _client = NULL;
    _server = NULL;
    _peer = NULL;
}

ENetHost *get_client_host(void)
{
    return _client;
}

ENetHost *get_server_host(void)
{
    return _server;
}

ENetPeer *get_peer(void)
{
    return _peer;
}

/******************************************************************************\
\******************************************************************************/

struct chunk_header *receive_chunk(ENetPacket *packet)
{
    size_t pos = 0;

    struct chunk_header *chunk = safe_malloc(packet->dataLength);
    if (!chunk)
        return NULL;

    chunk->type = read8(packet->data, &pos);

    switch (chunk->type)
    {
        case INPUT_CHUNK:
        {
            struct input_chunk *input = (void *)chunk;

            input->key = read8(packet->data, &pos);
            input->ms = read16(packet->data, &pos);

            break;
        }

        case CLIENT_MSG_CHUNK:
        {
            struct client_msg_chunk *client_msg = (void *)chunk;

            memcpy(client_msg->data, packet->data + pos, packet->dataLength - pos);

            break;
        }

        case LAND_CHUNK:
        {
            struct land_chunk *land = (void *)chunk;

            land->x = read16(packet->data, &pos);
            land->y = read16(packet->data, &pos);
            land->width = read16(packet->data, &pos);
            land->height = read16(packet->data, &pos);

            if (land->width < 0) land->width = 0;
            if (land->height < 0) land->height = 0;

            if (land->x < 0 || land->y < 0 ||
                land->x + land->width > LAND_WIDTH ||
                land->y + land->height > LAND_HEIGHT)
            {
                DIE("Bad LAND_CHUNK.");
            }

            if (pos == packet->dataLength)
            {
                DIE("Bad LAND_CHUNK (Zero-length).\n");
            }

            memcpy(land->data, packet->data + pos, packet->dataLength - pos);

            break;
        }

        case TANK_CHUNK:
        {
            struct tank_chunk *tank = (void *)chunk;

            tank->action = read8(packet->data, &pos);
            tank->id = read8(packet->data, &pos);
            tank->x = read16(packet->data, &pos);
            tank->y = read16(packet->data, &pos);
            tank->angle = read8(packet->data, &pos);

            break;
        }

        case BULLET_CHUNK:
        {
            struct bullet_chunk *bullet = (void *)chunk;

            bullet->action = read8(packet->data, &pos);
            bullet->id = read8(packet->data, &pos);
            bullet->x = read16(packet->data, &pos);
            bullet->y = read16(packet->data, &pos);

            break;
        }

        case CRATE_CHUNK:
        {
            struct crate_chunk *crate = (void *)chunk;

            crate->action = read8(packet->data, &pos);
            crate->x = read16(packet->data, &pos);
            crate->y = read16(packet->data, &pos);

            break;
        }

        case SERVER_MSG_CHUNK:
        {
            struct server_msg_chunk *server_msg = (void *)chunk;

            server_msg->id = read8(packet->data, &pos);
            server_msg->action = read8(packet->data, &pos);
            memcpy(server_msg->data, packet->data + pos, packet->dataLength - pos);

            break;
        }

        default:
            break;
    }

    return (void *)chunk;
}

void send_chunk(struct chunk_header *chunk, size_t len, bool broadcast, bool reliable)
{
    size_t pos = 0;
    uint8_t buffer[len];

    write8(buffer, &pos, chunk->type);

    switch (chunk->type)
    {
        case INPUT_CHUNK:
        {
            struct input_chunk *input = (void *)chunk;

            write8(buffer, &pos, input->key);
            write16(buffer, &pos, input->ms);

            break;
        }

        case CLIENT_MSG_CHUNK:
        {
            struct client_msg_chunk *client_msg = (void *)chunk;

            int end = len - pos;
            for (int i = 0; i < end; ++i)
                write8(buffer, &pos, client_msg->data[i]);

            break;
        }

        case LAND_CHUNK:
        {
            struct land_chunk *land = (void *)chunk;

            write16(buffer, &pos, land->x);
            write16(buffer, &pos, land->y);
            write16(buffer, &pos, land->width);
            write16(buffer, &pos, land->height);

            int end = len - pos;
            for (int i = 0; i < end; ++i)
                write8(buffer, &pos, land->data[i]);

            break;
        }

        case TANK_CHUNK:
        {
            struct tank_chunk *tank = (void *)chunk;

            write8(buffer, &pos, tank->action);
            write8(buffer, &pos, tank->id);
            write16(buffer, &pos, tank->x);
            write16(buffer, &pos, tank->y);
            write8(buffer, &pos, tank->angle);

            break;
        }

        case BULLET_CHUNK:
        {
            struct bullet_chunk *bullet = (void *)chunk;

            write8(buffer, &pos, bullet->action);
            write8(buffer, &pos, bullet->id);
            write16(buffer, &pos, bullet->x);
            write16(buffer, &pos, bullet->y);

            break;
        }

        case CRATE_CHUNK:
        {
            struct crate_chunk *crate = (void *)chunk;

            write8(buffer, &pos, crate->action);
            write16(buffer, &pos, crate->x);
            write16(buffer, &pos, crate->y);

            break;
        }

        case SERVER_MSG_CHUNK:
        {
            struct server_msg_chunk *server_msg = (void *)chunk;

            write8(buffer, &pos, server_msg->id);
            write8(buffer, &pos, server_msg->action);

            int end = len - pos;
            for (int i = 0; i < end; ++i)
                write8(buffer, &pos, server_msg->data[i]);

            break;
        }

        default:
            break;
    }

    send_packet(buffer, pos, broadcast, reliable);
}
