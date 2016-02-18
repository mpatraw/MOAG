
#ifndef BODY_HPP
#define BODY_HPP

#include "entity.hpp"

namespace m {

// A body for sprites/physics/rendering/etc.
class body : public component {
public:
    // Center pos.
    float x = 0.f;
    float y = 0.f;
    // Width/height of the object.
    float width = 0.f;
    float height = 0.f;

    body(entity *parent) : component{parent} {}
    virtual ~body() {}

    bool intersects(const body &other) const {
        bool a = std::abs(x - other.x) * 2 > width + other.width;
        bool b = std::abs(y - other.y) * 2 > height + other.height;
        return !a && !b;
    }
};

}

#endif
