#include <algorithm>
#include <memory>
#include <iostream>
#include <exception>

#include <enet/enet.h>

#include "error.hpp"
#include "network.hpp"

#if !defined(ENET_VERSION_MAJOR) || (ENET_VERSION_MAJOR == 1 && ENET_VERSION_MINOR == 2)
#define OLD_ENET
#endif

namespace m {

class network_impl {
public:

    network_impl(const char *ip, unsigned short port, int num_channels) {
        if (enet_initialize() != 0) {
            throw initialization_error("could not initialize enet");
        }
#ifdef OLD_ENET
        host = enet_host_create(NULL, 1, 0, 0);
#else
        host = enet_host_create(NULL, 1, num_channels, 0, 0);
#endif
        if (!host) {
            throw initialization_error("could not create client host");
        }

        ENetAddress address;
        enet_address_set_host(&address, ip);
        address.port = port;

        peers.resize(1, nullptr);
#ifdef OLD_ENET
        peers[0] = enet_host_connect(host, &address, num_channels);
#else
        peers[0] = enet_host_connect(host, &address, num_channels, 0);
#endif
        if (!peers[0]) {
            throw initialization_error("no available peers");
        }

        ENetEvent ev;
        if (enet_host_service(host, &ev, connect_timeout) == 0 ||
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

        peers.resize(max_connections, nullptr);
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

    packet recv() {
        ENetEvent event;
        if (enet_host_service(host, &event, 10)) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    int id = 0;
                    if (is_server) {
                        size_t i;
                        for (i = 0; i < peers.size(); ++i) {
                            if (!peers[i]) {
                                break;
                            }
                        }
                        event.peer->data = reinterpret_cast<void *>(i);
                        peers[i] = event.peer;
                        id = i;
                    }
                    return packet{packet_type::connection, id};
                }

                case ENET_EVENT_TYPE_DISCONNECT: {
                    int id = 0;
                    if (is_server) {
                        size_t i = reinterpret_cast<size_t>(event.peer->data);
                        peers[i] = nullptr;
                        id = i;
                    }
                    return packet{packet_type::disconnection, id};
                }

                case ENET_EVENT_TYPE_RECEIVE: {
                    int id = 0;
                    if (is_server) {
                        size_t i = reinterpret_cast<size_t>(event.peer->data);
                        id = static_cast<int>(i);
                    }
                    serializer s{event.packet->data, event.packet->dataLength, true};
                    packet p{packet_type::message, id};
                    s.decompress();
                    p.get_message().serialize(s);
                    enet_packet_destroy(event.packet);
                    return p;
                }

                default:
                    break;
            }
        }
        return packet{};
    }

    void send(message &msg) {
        serializer s;
        msg.serialize(s);
        s.compress();
        uint32_t flags = 0;
        if (msg.should_be_reliable()) {
            flags |= ENET_PACKET_FLAG_RELIABLE;
        }
        ENetPacket *packet = enet_packet_create(NULL, s.size(), flags);
        if (!packet) {
            throw std::bad_alloc();
        }
        std::copy_n(s.data(), s.size(), packet->data);
        if (is_server) {
            enet_host_broadcast(host, 1, packet);
        } else {
            enet_peer_send(peers[0], 1, packet);
        }
    }

    uint32_t rtt(int id=0) const {
        return peers[id]->roundTripTime;
    }

private:
    ENetHost *host;
    std::vector<ENetPeer *> peers;
    bool is_server;
};


network_manager::network_manager(const char *ip, unsigned short port, int num_channels) :
    impl{std::make_unique<network_impl>(ip, port, num_channels)} {
}

network_manager::network_manager(unsigned short port, int max_connections, int num_channels) :
    impl{std::make_unique<network_impl>(port, max_connections, num_channels)} {
}

network_manager::~network_manager() {}

packet network_manager::recv() {
    return impl->recv();
}

void network_manager::send(message &msg) {
    impl->send(msg);
}

uint32_t network_manager::rtt() const {
    return impl->rtt();
}

}
