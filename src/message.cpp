
#include <cassert>
#include <functional>
#include <map>

#include "message.hpp"

namespace m {

// I wish there was a better way?
static std::map<message_type, std::function<message_def *(serializer &)>> message_lookup = {
    {
        message_type::input,
        [](serializer &s) -> message_def * {
            auto m = new input_message_def;
            m->serialize(s);
            return m;
        },
    },
    {
        message_type::message,
        [](serializer &s) -> message_def * {
            auto m = new message_message_def;
            m->serialize(s);
            return m;
        },
    },
    {
        message_type::land,
        [](serializer &s) -> message_def * {
            auto m = new land_message_def;
            m->serialize(s);
            return m;
        },
    },
    {
        message_type::tank,
        [](serializer &s) -> message_def * {
            auto m = new tank_message_def;
            m->serialize(s);
            return m;
        },
    },
    {
        message_type::bullet,
        [](serializer &s) -> message_def * {
            auto m = new bullet_message_def;
            m->serialize(s);
            return m;
        },
    },
    {
        message_type::crate,
        [](serializer &s) -> message_def * {
            auto m = new crate_message_def;
            m->serialize(s);
            return m;
        },
    },
};

message::message(serializer &s) {
    assert(s.is_deserializing());
    serialize(s);
}

message::message(message_def *mdef) :
    type_{mdef->get_type()}, def_{mdef} {
}

void message::set_def(message_def *mdef) {
    type_ = mdef->get_type();
    def_.reset(mdef);
}

message_def &message::get_def() {
    return *def_.get();
}

const message_def &message::get_def() const {
    return *def_.get();
}

message_type message::get_type() const {
    return type_;
}

void message::serialize(serializer &s) {
    uint8_t tmp = static_cast<uint8_t>(type_);
    s & tmp;
    type_ = static_cast<message_type>(tmp);
    if (s.is_serializing()) {
        def_->serialize(s);
    } else {
        def_.reset(message_lookup[type_](s));
    }
}

}

