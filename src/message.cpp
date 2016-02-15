
#include <functional>
#include <map>

#include "message.hpp"

namespace m {

static std::map<uint8_t, std::function<message *(serializer &)>> message_lookup = {
    {
        static_cast<uint8_t>(message_type::tank),
        [](serializer &s) -> message * {
            auto m = new tank_message;
            m->serialize(s);
            return m;
        }
    },
};

std::shared_ptr<message> deserialize_message(serializer &s) {
    uint8_t type;
    s & type;
    return std::shared_ptr<message>(message_lookup[type](s));
}

}

