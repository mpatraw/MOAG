
#include <map>

#include "message.hpp"

namespace m {

typedef message *create(serializer &s);

static std::map<message_type, create *> message_lookup = {
    {
        message_type::tank,
        [](serializer &s) -> message * {
            auto m = new tank_message;
            m->serialize(s);
            return m;
        }
    },
};

std::shared_ptr<message> deserialize_message(serializer &s) {
    message_type type;
    s & type;
    return std::shared_ptr<message>(message_lookup[type](s));
}

}

