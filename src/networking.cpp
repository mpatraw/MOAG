#include <exception>

#include "moag.hpp"

#if !defined(ENET_VERSION_MAJOR) || (ENET_VERSION_MAJOR == 1 && ENET_VERSION_MINOR == 2)
#define OLD_ENET
#endif

namespace m {

class network_impl {
public:
    network_impl(const char *ip, unsigned short port,
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

        peers.reserve(1);
#ifdef OLD_ENET
        peers[0] = enet_host_connect(host, &address, num_channels);
#else
        peers[0] = enet_host_connect(host, &address, num_channels, 0);
#endif
        if (!peers[0]) {
            throw initialization_error("no available peers");
        }

        ENetEvent ev;
        if (enet_host_service(host, &ev, g_connect_timeout) == 0 ||
            ev.type != ENET_EVENT_TYPE_CONNECT) {
            enet_peer_reset(peers[0]);
            throw initialization_error("connection timed out");
        }

        is_server = false;
    }
    network_impl(unsigned short port, int max_connections, int num_channels) {
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

        peers.reserve(max_connections);
        is_server = true;
    }
    ~network_impl() {
        if (!is_server) {
            enet_peer_disconnect(peers[0], 0);
            enet_peer_reset(peers[0]);
        }
        enet_host_destroy(host);
        enet_deinitialize();
    }

    bool is_connected(int id=0) const {
        return peers[id] != nullptr;
    }

    packet &recv(int &id) {
        current_packet.reread();
        current_packet.rewrite();
        ENetEvent event;
        if (enet_host_service(host, &event, 10))
        {
            switch (event.type)
            {
                case ENET_EVENT_TYPE_CONNECT: {
                    if (is_server) {
                        size_t i;
                        for (i = 0; i < peers.size(); ++i) {
                            if (!peers[i]) {
                                break;
                            }
                        }
                        event.peer->data = static_cast<void *>(&i);
                        peers[i] = event.peer;
                        id = i;
                    }
                    current_packet << packet_type_connection;
                    break;
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    if (is_server) {
                        size_t i = *static_cast<size_t *>(event.peer->data);
                        peers[i] = nullptr;
                        id = i;
                    }
                    current_packet << packet_type_disconnection;
                    break;
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    if (is_server) {
                        size_t i = *static_cast<size_t *>(event.peer->data);
                        id = static_cast<int>(i);
                    }
                    current_packet.load(event.packet->data, event.packet->dataLength);
                    enet_packet_destroy(event.packet);
                    break;
                }

                default:
                    break;
            }
        }
        return current_packet;
    }

    void send(const packet &p, int id, bool reliable=true) {
        uint32_t flags = 0;
        if (reliable) {
            flags |= ENET_PACKET_FLAG_RELIABLE;
        }
        ENetPacket *packet = enet_packet_create(NULL, p.size() + 2, flags);
        if (!packet) {
            throw std::bad_alloc();
        }
        std::copy_n(p.data(), p.size(), packet->data);
        enet_peer_send(peers[id], 1, packet);
    }

    void broadcast(const packet &p, bool reliable=true) {
        assert(is_server);
        uint32_t flags = 0;
        if (reliable) {
            flags |= ENET_PACKET_FLAG_RELIABLE;
        }
        ENetPacket *packet = enet_packet_create(NULL, p.size(), flags);
        if (!packet) {
            throw std::bad_alloc();
        }
        std::copy_n(p.data(), p.size(), packet->data);
        enet_host_broadcast(host, 1, packet);
    }

    uint32_t rtt(int id=0) const {
        return peers[id]->roundTripTime;
    }

private:
    ENetHost *host;
    std::vector<ENetPeer *> peers;
    packet current_packet;
    bool is_server;
};

client::client() :
    impl{std::make_unique<network_impl>(g_host, g_port, g_max_players, g_number_of_channels)} {
}

client::~client() { }

packet &client::recv() {
    int _;
    return impl->recv(_);
}

void client::send(const packet &p, bool reliable) {
    impl->send(p, 0, reliable);
}

uint32_t client::rtt() const {
    return impl->rtt();
}

server::server() :
    impl{std::make_unique<network_impl>(g_port, g_max_players, g_number_of_channels)} {
}

server::~server() {
}

bool server::is_connected(int id) const {
    return impl->is_connected(id);
}

packet &server::recv(int &id) {
    return impl->recv(id);
}

void server::send(const packet &p, int id, bool reliable) {
    impl->send(p, id, reliable);
}

void server::broadcast(const packet &p, bool reliable) {
    impl->broadcast(p, reliable);
}

uint32_t server::rtt(int id) const {
    return impl->rtt(id);
}

}