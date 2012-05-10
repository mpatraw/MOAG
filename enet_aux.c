
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#include "enet_aux.h"

static void die(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

bool _initialized = false;

static ENetHost *_client = NULL;
static ENetHost *_server = NULL;
static ENetPeer *_peer = NULL;

void init_enet_client(const char *ip, unsigned port)
{
    if (!_initialized) {
        if (enet_initialize() != 0)
            die("An error occurred while initializing ENet.\n");
        else
            atexit(enet_deinitialize);
    }
    _initialized = true;

    _client = enet_host_create(NULL, MAX_CLIENTS, NUM_CHANNELS, 0, 0);
    if (!_client)
        die("An error occurred while trying to create an ENet client host.\n");

    ENetAddress address;
    enet_address_set_host(&address, ip);
    address.port = port;

    _peer = enet_host_connect(_client, &address, NUM_CHANNELS, 0);
    if (!_peer)
        die("No available peers for initiating an ENet connection.\n");

    ENetEvent ev;

    if (enet_host_service(_client, &ev, CONNECT_TIMEOUT) == 0 ||
        ev.type != ENET_EVENT_TYPE_CONNECT) {
        enet_peer_reset(_peer);
        die("Connection to %s timed out.\n", ip);
    }
}

void init_enet_server(unsigned port)
{
    if (!_initialized) {
        if (enet_initialize() != 0)
            die("An error occurred while initializing ENet.\n");
        else
            atexit(enet_deinitialize);
    }
    _initialized = true;

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    _server = enet_host_create(&address, MAX_CLIENTS, NUM_CHANNELS, 0, 0);
    if (!_server)
        die("An error occurred while trying to create an ENet server host.\n");

    fprintf(stdout, "Started server\n");
}

void uninit_enet(void)
{
    if (_peer) {
        enet_peer_disconnect(_peer, 0);

        ENetEvent ev;
        while (enet_host_service(_client, &ev, 3000)) {
            switch (ev.type) {
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
