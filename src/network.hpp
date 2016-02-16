
#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "message.hpp"

namespace m {

const int connect_timeout = 10000;

class network_impl;

enum class packet_type {
    none,
    connection,
    disconnection,
    message
};

class packet final {
public:
    packet() {}
    packet(packet_type type, int id) : type_{type}, id_{id} {}

    packet_type get_type() const { return type_; }
    int get_id() const { return id_; }
    const message &get_message() const { return msg_; }
    message &get_message() { return msg_; }

private:
    packet_type type_ = packet_type::none;
    message msg_;
    int id_;
};

class network_manager final {
public:
    // Client constructor.
    network_manager(const char *ip, unsigned short port, int num_channels);
    // Server constructor.
    network_manager(unsigned short port, int max_connections, int num_channels);
    ~network_manager();

    packet recv();
    void send(message &msg, bool reliable = true);

    uint32_t rtt() const;
private:
    std::unique_ptr<network_impl> impl;
};

}

#endif
