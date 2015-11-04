
#include <errno.h>
#include <zlib.h>

#include "common.hpp"

/******************************************************************************\
\******************************************************************************/

// convenience hack for old enet support
#if !defined(ENET_VERSION_MAJOR) || (ENET_VERSION_MAJOR == 1 && ENET_VERSION_MINOR == 2)
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
        if (enet_initialize() != 0) {
            fprintf(stderr, "could not initialize enet\n");
            exit(-1);
    } else {
            atexit(enet_deinitialize);
    }
    }
    _initialized = true;

#ifdef OLD_ENET
    _client = enet_host_create(NULL, g_max_players, 0, 0);
#else
    _client = enet_host_create(NULL, g_max_players, g_number_of_channels, 0, 0);
#endif
    if (!_client) {
        fprintf(stderr, "could not initialize enet\n");
        exit(-1);
    }

    ENetAddress address;
    enet_address_set_host(&address, ip);
    address.port = port;

#ifdef OLD_ENET
    _peer = enet_host_connect(_client, &address, g_number_of_channels);
#else
    _peer = enet_host_connect(_client, &address, g_number_of_channels, 0);
#endif
    if (!_peer) {
        fprintf(stderr, "no available peers\n");
        exit(-1);
    }

    ENetEvent ev;

    if (enet_host_service(_client, &ev, g_connect_timeout) == 0 ||
        ev.type != ENET_EVENT_TYPE_CONNECT)
    {
        enet_peer_reset(_peer);
        fprintf(stderr, "connection to %s timed out\n", ip);
        exit(-1);
    }
}

void init_enet_server(unsigned port)
{
    if (!_initialized)
    {
        if (enet_initialize() != 0) {
            fprintf(stderr, "error occured initializing enet\n");
            exit(-1);
    } else {
            atexit(enet_deinitialize);
    }
    }
    _initialized = true;

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

#ifdef OLD_ENET
    _server = enet_host_create(&address, g_max_players, 0, 0);
#else
    _server = enet_host_create(&address, g_max_players, g_number_of_channels, 0, 0);
#endif
    if (!_server) {
        fprintf(stderr, "an error occurred while trying to create an ENet server host.\n");
    exit(-1);
    }
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

    struct chunk_header *chunk = (struct chunk_header *)malloc(packet->dataLength);
    if (!chunk)
        return NULL;

    chunk->type = read8(packet->data, &pos);

    switch (chunk->type)
    {
        case INPUT_CHUNK:
        {
            struct input_chunk *input = (struct input_chunk *)chunk;

            input->key = read8(packet->data, &pos);
            input->ms = read16(packet->data, &pos);

            break;
        }

        case CLIENT_MSG_CHUNK:
        {
            struct client_msg_chunk *client_msg = (struct client_msg_chunk *)chunk;

            memcpy(client_msg->data, packet->data + pos, packet->dataLength - pos);

            break;
        }

        case LAND_CHUNK:
        {
            struct land_chunk *land = (struct land_chunk *)chunk;

            land->x = read16(packet->data, &pos);
            land->y = read16(packet->data, &pos);
            land->width = read16(packet->data, &pos);
            land->height = read16(packet->data, &pos);

            if (land->width < 0) land->width = 0;
            if (land->height < 0) land->height = 0;

            if (land->x < 0 || land->y < 0 ||
                land->x + land->width > g_land_width ||
                land->y + land->height > g_land_height)
            {
                fprintf(stderr, "bad chunk\n");
        exit(-1);
            }

            if (pos == packet->dataLength)
            {
                fprintf(stderr, "Bad LAND_CHUNK (Zero-length).\n");
                exit(-1);
            }

            memcpy(land->data, packet->data + pos, packet->dataLength - pos);

            break;
        }

        case PACKED_LAND_CHUNK:
        {
            struct packed_land_chunk *land = (struct packed_land_chunk *)chunk;

            land->x = read16(packet->data, &pos);
            land->y = read16(packet->data, &pos);
            land->width = read16(packet->data, &pos);
            land->height = read16(packet->data, &pos);

            if (land->width < 0) land->width = 0;
            if (land->height < 0) land->height = 0;

            if (land->x < 0 || land->y < 0 ||
                land->x + land->width > g_land_width ||
                land->y + land->height > g_land_height)
            {
                fprintf(stderr, "Bad LAND_CHUNK.\n");
                exit(-1);
            }

            if (pos == packet->dataLength)
            {
                fprintf(stderr, "Bad LAND_CHUNK (Zero-length).\n");
                exit(-1);
            }

            memcpy(land->data, packet->data + pos, packet->dataLength - pos);

            break;
        }

        case TANK_CHUNK:
        {
            struct tank_chunk *tank = (struct tank_chunk *)chunk;

            tank->action = read8(packet->data, &pos);
            tank->id = read8(packet->data, &pos);
            tank->x = read16(packet->data, &pos);
            tank->y = read16(packet->data, &pos);
            tank->angle = read8(packet->data, &pos);

            break;
        }

        case BULLET_CHUNK:
        {
            struct bullet_chunk *bullet = (struct bullet_chunk *)chunk;

            bullet->action = read8(packet->data, &pos);
            bullet->id = read8(packet->data, &pos);
            bullet->x = read16(packet->data, &pos);
            bullet->y = read16(packet->data, &pos);

            break;
        }

        case CRATE_CHUNK:
        {
            struct crate_chunk *crate = (struct crate_chunk *)chunk;

            crate->action = read8(packet->data, &pos);
            crate->x = read16(packet->data, &pos);
            crate->y = read16(packet->data, &pos);

            break;
        }

        case SERVER_MSG_CHUNK:
        {
            struct server_msg_chunk *server_msg = (struct server_msg_chunk *)chunk;

            server_msg->id = read8(packet->data, &pos);
            server_msg->action = read8(packet->data, &pos);
            memcpy(server_msg->data, packet->data + pos, packet->dataLength - pos);

            break;
        }

        default:
            break;
    }

    return (struct chunk_header *)chunk;
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
            struct input_chunk *input = (struct input_chunk *)chunk;

            write8(buffer, &pos, input->key);
            write16(buffer, &pos, input->ms);

            break;
        }

        case CLIENT_MSG_CHUNK:
        {
            struct client_msg_chunk *client_msg = (struct client_msg_chunk *)chunk;

            int end = len - pos;
            for (int i = 0; i < end; ++i)
                write8(buffer, &pos, client_msg->data[i]);

            break;
        }

        case LAND_CHUNK:
        {
            struct land_chunk *land = (struct land_chunk *)chunk;

            write16(buffer, &pos, land->x);
            write16(buffer, &pos, land->y);
            write16(buffer, &pos, land->width);
            write16(buffer, &pos, land->height);

            int end = len - pos;
            for (int i = 0; i < end; ++i)
                write8(buffer, &pos, land->data[i]);

            break;
        }

        case PACKED_LAND_CHUNK:
        {
            struct packed_land_chunk *land = (struct packed_land_chunk *)chunk;

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
            struct tank_chunk *tank = (struct tank_chunk *)chunk;

            write8(buffer, &pos, tank->action);
            write8(buffer, &pos, tank->id);
            write16(buffer, &pos, tank->x);
            write16(buffer, &pos, tank->y);
            write8(buffer, &pos, tank->angle);

            break;
        }

        case BULLET_CHUNK:
        {
            struct bullet_chunk *bullet = (struct bullet_chunk *)chunk;

            write8(buffer, &pos, bullet->action);
            write8(buffer, &pos, bullet->id);
            write16(buffer, &pos, bullet->x);
            write16(buffer, &pos, bullet->y);

            break;
        }

        case CRATE_CHUNK:
        {
            struct crate_chunk *crate = (struct crate_chunk *)chunk;

            write8(buffer, &pos, crate->action);
            write16(buffer, &pos, crate->x);
            write16(buffer, &pos, crate->y);

            break;
        }

        case SERVER_MSG_CHUNK:
        {
            struct server_msg_chunk *server_msg = (struct server_msg_chunk *)chunk;

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
