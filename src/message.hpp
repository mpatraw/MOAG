
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "serialize.hpp"

namespace m {

enum class message_type : uint8_t {
    input,
    message,
    land,
    tank,
    bullet,
    crate
};

struct message_def : public serializable {
    virtual ~message_def() {}
    virtual message_type get_type() const = 0;
};

struct input_message_def : public message_def {
    uint8_t key;
    uint16_t ms;

    input_message_def() {}
    input_message_def(uint8_t key, uint16_t ms) : key{key}, ms{ms} {}
    virtual ~input_message_def() {}
    message_type get_type() const override { return message_type::input; }

    void serialize(serializer &s) override {
        s & key;
        s & ms;
    }
};

struct message_message_def : public message_def {
    uint8_t action, id;
    std::vector<int8_t> msg;

    message_message_def() {}
    message_message_def(uint8_t action, uint8_t id, const std::string &s) :
        action{action}, id{id}, msg{s.begin(), s.end()} {}
    virtual ~message_message_def() {}
    message_type get_type() const override { return message_type::message; }

    void serialize(serializer &s) override {
        s & action;
        s & id;
        uint32_t len = msg.size();
        s & len;
        msg.resize(len);
        for (size_t i = 0; i < len; ++i) {
            s & msg[i]; 
        }
    }

    void set_string(std::string s) {
        msg = std::vector<int8_t>{s.begin(), s.end()};
    }
    std::string get_string() const {
        return std::string(msg.begin(), msg.end());
    }
};

struct land_message_def : public message_def {
    uint16_t x, y, w, h;
    std::vector<uint8_t> data;

    land_message_def() {}
    land_message_def(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const std::vector<uint8_t> &data) :
        x{x}, y{y}, w{w}, h{h}, data{data} {}
    virtual ~land_message_def() {}
    message_type get_type() const override { return message_type::land; }

    void serialize(serializer &s) override {
        s & x;
        s & y;
        s & w;
        s & h;
        data.resize(w * h);
        size_t i = 0;
        for (size_t yy = y; yy < h + y; ++yy) {
            for (size_t xx = x; xx < w + x; ++xx) {
                (void)yy;
                (void)xx;
                s & data[i++];
            }
        }
    }
};

struct tank_message_def : public message_def {
    uint8_t action, id, angle;
    uint16_t x, y;

    tank_message_def() {}
    tank_message_def(uint8_t action, uint8_t id, uint8_t angle, uint16_t x, uint16_t y) :
        action{action}, id{id}, angle{angle}, x{x}, y{y} {}
    virtual ~tank_message_def() {}
    message_type get_type() const override { return message_type::tank; }

    void serialize(serializer &s) override {
        s & action;
        s & id;
        s & x;
        s & y;
        s & angle;
    }
};

struct bullet_message_def : public message_def {
    uint8_t action, id;
    uint16_t x, y;

    bullet_message_def() {}
    bullet_message_def(uint8_t action, uint8_t id, uint16_t x, uint16_t y) :
        action{action}, id{id}, x{x}, y{y} {}
    virtual ~bullet_message_def() {}
    message_type get_type() const override { return message_type::bullet; }

    void serialize(serializer &s) override {
        s & action;
        s & id;
        s & x;
        s & y;
    }
};

struct crate_message_def : public message_def {
    uint8_t action;
    uint16_t x, y;

    crate_message_def() {}
    crate_message_def(uint8_t a, uint16_t x, uint16_t y) :
        action{a}, x{x}, y{y} {}
    virtual ~crate_message_def() {}
    message_type get_type() const override { return message_type::crate; }

    void serialize(serializer &s) override {
        s & action;
        s & x;
        s & y;
    }
};

// The root message. Holds a type and the definition.
class message : public serializable {
public:
    message() {}
    message(serializer &s);
    message(message_def *mdef);
    message(message &&) = default;
    message &operator =(message &&) = default;
    virtual ~message() { }

    void serialize(serializer &s) override;

    void set_def(message_def *mdef);
    message_def &get_def();
    const message_def &get_def() const;
    message_type get_type() const;

private:
    message_type type_;
    std::unique_ptr<message_def> def_;
};

}

#endif

