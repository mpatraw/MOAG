#include <exception>

#include "moag.hpp"

#if !defined(ENET_VERSION_MAJOR) || (ENET_VERSION_MAJOR == 1 && ENET_VERSION_MINOR == 2)
#define OLD_ENET
#endif

namespace m {

class client_impl {
public:
    client_impl(const char *ip, unsigned short port,
                int max_connections, int num_channels) {
        if (enet_initialize() != 0) {
            throw initialization_error("could not initialize enet");
        }
#ifdef OLD_ENET
        host = enet_host_create(NULL, max_connections, 0, 0);
#else
        host = enet_host_create(NULL, max_connections, num_channels, 0, 0);
#endif
        if (!host) {
            throw initialization_error("could not create client host");
        }

        ENetAddress address;
        enet_address_set_host(&address, ip);
        address.port = port;
#ifdef OLD_ENET
        peer = enet_host_connect(host, &address, num_channels);
#else
        peer = enet_host_connect(host, &address, num_channels, 0);
#endif
        if (!peer) {
            throw initialization_error("no available peers");
        }

        ENetEvent ev;
        if (enet_host_service(host, &ev, g_connect_timeout) == 0 ||
            ev.type != ENET_EVENT_TYPE_CONNECT) {
            enet_peer_reset(peer);
            throw initialization_error("connection timed out");
        }
    }
    ~client_impl() {
        enet_peer_disconnect(peer, 0);
        enet_peer_reset(peer);
        enet_host_destroy(host);
        enet_deinitialize();
    }

    packet &recv() {
        current_packet.reread();
        current_packet.rewrite();
        ENetEvent event;
        if (enet_host_service(host, &event, 10))
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT:
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    current_packet.load(event.packet->data, event.packet->dataLength);
                    enet_packet_destroy(event.packet);
                    break;

                default:
                    break;
            }
        }
        return current_packet;
    }

    void send(const packet &p, bool reliable=true) {
        uint32_t flags = 0;
        if (reliable) {
            flags |= ENET_PACKET_FLAG_RELIABLE;
        }
        ENetPacket *packet = enet_packet_create(NULL, p.size(), flags);
        if (!packet) {
            throw std::bad_alloc();
        }
        std::copy_n(p.data(), p.size(), packet->data);
        enet_peer_send(peer, 1, packet);
    }

private:
    ENetHost *host;
    ENetPeer *peer;
    packet current_packet;
};

class server_impl {
public:
    server_impl(unsigned short port, int max_connections, int num_channels) {
        if (enet_initialize() != 0) {
            throw initialization_error("could not initialize enet");
        }

        ENetAddress address;
        address.host = ENET_HOST_ANY;
        address.port = port;

#ifdef OLD_ENET
        host = enet_host_create(&address, max_connections, 0, 0);
#else
        host = enet_host_create(&address, max_connections, num_channels, 0, 0);
#endif
        if (!host) {
            throw initialization_error("could not create server host");
        }
    }
    ~server_impl() {
        enet_host_destroy(host);
        enet_deinitialize();
    }
private:
    ENetHost *host;
};

client::client() :
    impl{std::make_unique<client_impl>(g_host, g_port, g_max_players, g_number_of_channels)} {
}

client::~client() { }

packet &client::recv() {
    return impl->recv();
}

void client::send(const packet &p) {
    impl->send(p);
}

server::server() :
    impl{std::make_unique<server_impl>(g_port, g_max_players, g_number_of_channels)} {
}

}
