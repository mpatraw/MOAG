
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>
#include <memory>
#include <vector>

#include "serialize.hpp"

namespace m {

enum class message_type {
    tank
};

struct message {
    virtual ~message() {}
};

struct input_message : public message, serializable {
    uint8_t key;
    uint16_t ms;

    input_message() {}
    virtual ~input_message() {}

    void serialize(serializer &s) override {
        s & key;
        s & ms;
    }
};

struct message_message : public message, serializable {
    size_t len;
    std::vector<uint8_t> msg;

    message_message() {}
    virtual ~message_message() {}

    void serialize(serializer &s) override {
        len = msg.size();
        s & len;
        msg.resize(len);
        for (size_t i = 0; i < len; ++i) {
            s & msg[i]; 
        }
    }
};

struct land_message : public message, serializable {
    uint16_t x, y, w, h;
    std::vector<uint8_t> data;

    land_message() {}
    virtual ~land_message() {}

    void serialize(serializer &s) override {
        s & x;
        s & y;
        s & w;
        s & h;
        data.resize(w * h);
        size_t i = 0;
        for (size_t yy = y; y < h + y; ++y) {
            for (size_t xx = x; x < w + x; ++x) {
                (void)yy;
                (void)xx;
                s & data[i++];
            }
        }
    }
};

struct tank_message : public message, serializable {
    uint8_t action, id, angle;
    uint16_t x, y;

    tank_message() {}
    virtual ~tank_message() {}

    void serialize(serializer &s) override {
        s & action;
        s & id;
        s & x;
        s & y;
        s & angle;
    }
};

struct bullet_message : public message, serializable {
    uint8_t action, id;
    uint16_t x, y;

    bullet_message() {}
    virtual ~bullet_message() {}

    void serialize(serializer &s) override {
        s & action;
        s & id;
        s & x;
        s & y;
    }
};

struct crate_message : public message, serializable {
    uint8_t action;
    uint16_t x, y;

    crate_message() {}
    virtual ~crate_message() {}

    void serialize(serializer &s) override {
        s & action;
        s & x;
        s & y;
    }
};


std::shared_ptr<message> deserialize_message(serializer &s);
    
}

#endif

