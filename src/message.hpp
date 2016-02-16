
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "serialize.hpp"

namespace m {

enum class entity_op_type : uint8_t {
    spawn,
    kill,
    move
};

enum class message_op_type : uint8_t {
    client,
    chat,
    server_notice,
    name_change
};

enum class tank_facing_type : uint8_t {
    right,
    left
};

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
    virtual bool should_be_reliable() const = 0;
};

struct input_message_def : public message_def {
    uint8_t key;
    uint16_t ms;

    input_message_def() {}
    input_message_def(uint8_t key, uint16_t ms) : key{key}, ms{ms} {}
    virtual ~input_message_def() {}
    message_type get_type() const override { return message_type::input; }
    bool should_be_reliable() const override { return true; }

    void serialize(serializer &s) override {
        s & key;
        s & ms;
    }
};

struct message_message_def : public message_def {
    message_op_type op;
    uint8_t id;
    std::vector<int8_t> msg;

    message_message_def() {}
    message_message_def(message_op_type op, uint8_t id, const std::string &s) :
        op{op}, id{id}, msg{s.begin(), s.end()} {}
    virtual ~message_message_def() {}
    message_type get_type() const override { return message_type::message; }
    bool should_be_reliable() const override { return true; }

    void serialize(serializer &s) override {
        uint8_t o = static_cast<uint8_t>(op);
        s & o;
        op = static_cast<message_op_type>(o);
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
    bool should_be_reliable() const override { return true; }

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
    entity_op_type op;
    tank_facing_type facing;
    uint8_t id, angle;
    uint16_t x, y;

    tank_message_def() {}
    tank_message_def(entity_op_type op, tank_facing_type facing, uint8_t id, uint8_t angle, uint16_t x, uint16_t y) :
        op{op}, facing{facing}, id{id}, angle{angle}, x{x}, y{y} {}
    virtual ~tank_message_def() {}
    message_type get_type() const override { return message_type::tank; }
    bool should_be_reliable() const override {
        return op == entity_op_type::spawn || op == entity_op_type::kill;
    }

    void serialize(serializer &s) override {
        uint8_t o = static_cast<uint8_t>(op);
        s & o;
        op = static_cast<entity_op_type>(o);
        uint8_t face = static_cast<uint8_t>(facing);
        s & face;
        facing = static_cast<tank_facing_type>(face);
        s & id;
        s & angle;
        s & x;
        s & y;
    }
};

struct bullet_message_def : public message_def {
    entity_op_type op;
    uint8_t id;
    uint16_t x, y;

    bullet_message_def() {}
    bullet_message_def(entity_op_type op, uint8_t id, uint16_t x, uint16_t y) :
        op{op}, id{id}, x{x}, y{y} {}
    virtual ~bullet_message_def() {}
    message_type get_type() const override { return message_type::bullet; }
    bool should_be_reliable() const override {
        return op == entity_op_type::spawn || op == entity_op_type::kill;
    }

    void serialize(serializer &s) override {
        uint8_t o = static_cast<uint8_t>(op);
        s & o;
        op = static_cast<entity_op_type>(o);
        s & id;
        s & x;
        s & y;
    }
};

struct crate_message_def : public message_def {
    entity_op_type op;
    uint16_t x, y;

    crate_message_def() {}
    crate_message_def(entity_op_type op, uint16_t x, uint16_t y) :
        op{op}, x{x}, y{y} {}
    virtual ~crate_message_def() {}
    message_type get_type() const override { return message_type::crate; }
    bool should_be_reliable() const override {
        return op == entity_op_type::spawn || op == entity_op_type::kill;
    }

    void serialize(serializer &s) override {
        uint8_t o = static_cast<uint8_t>(op);
        s & o;
        op = static_cast<entity_op_type>(o);
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
    bool should_be_reliable() const;

private:
    message_type type_;
    std::unique_ptr<message_def> def_;
};

}

#endif

