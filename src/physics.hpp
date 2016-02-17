
#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <cmath>
#include <functional>
#include <memory>
#include <tuple>

#include "land.hpp"
#include "line_path.hpp"

namespace m {

const float gravity = 1;

// Axis-aligned bounding box.
struct aabb {
    // Center.
    float x, y;
    float w, h;

    std::pair<float, float> top_left() const { return std::make_pair(x - w / 2, y - h / 2); }
    std::pair<float, float> top_right() const { return std::make_pair(x + w / 2, y - h / 2); }
    std::pair<float, float> bottom_left() const { return std::make_pair(x - w / 2, y + h / 2); }
    std::pair<float, float> bottom_right() const { return std::make_pair(x + w / 2, y + h / 2); }

    bool colliding_with(const aabb &other) const {
        bool a = std::abs(x - other.x) * 2 > w + other.w;
        bool b = std::abs(y - other.y) * 2 > h + other.h;
        return a && b;
    }
};

struct body {
    float vx, vy;
    aabb bounds;
    // If two masks equal something other than 0 when bitwise-anded together,
    // they collide.
    unsigned int collision_mask;

    std::function<void(std::shared_ptr<body>)> hit_bod;
    std::function<void(int x, int y)> hit_land;
};

class physics final {
public:
    physics(const land &l) : main_land{l} {}

    void step(std::shared_ptr<body> bod, uint32_t dt) {
        float sec = dt / 1000.f;
        float tx = bod->bounds.x + bod->vx * sec;
        float ty = bod->bounds.y + bod->vy * sec;
        // Apply gravity after applying velocity to allow movement when
        // velocity is set manually.
        bod->vy += gravity;

        line_path<> lp{
            static_cast<int>(bod->bounds.x), static_cast<int>(bod->bounds.y),
            static_cast<int>(tx),  static_cast<int>(ty)};

        // No movement.
        if (lp.length() == 1) {
            bod->bounds.x = tx;
            bod->bounds.y = ty;
            return;
        }

        for (const auto &p : lp) {
            int px, py;
            std::tie(px, py) = p;
            if (main_land.is_dirt(px, py)) {
                if (bod->hit_land) {
                    bod->hit_land(px, py);
                }
                break;
            } else {
                bod->bounds.x = static_cast<float>(px);
                bod->bounds.y = static_cast<float>(py);
            }
        }

    }

    void step(uint32_t dt) {
        auto i = std::begin(bodies);
        while (i != std::end(bodies)) {
            auto bp = i->lock();
            if (!bp) {
                i = bodies.erase(i);
                continue;
            }
            step(bp, dt);
            ++i;
        }
    }

    void append_body(std::weak_ptr<body> bod) {
        bodies.push_back(bod);
    }

private:
    const land &main_land;
    std::vector<std::weak_ptr<body>> bodies;
};

}

#endif

