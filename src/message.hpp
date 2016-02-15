
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>
#include <memory>

#include "serialize.hpp"

namespace m {

struct message {};

struct tank_message : public message, serializable {
    tank_message();

    void serialize(serializer &s) override;
};

std::shared_ptr<message> deserialize_message(serializer &s);
    
}

#endif

