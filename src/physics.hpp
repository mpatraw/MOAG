
#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "land.hpp"

namespace m {

struct body {
    float x, y;
    float vx, vy;
};

class physics final {
public:
    physics(const land &l) : main_land{l} {}

    void step(std::shared_ptr<body> bod, uint32_t dt) {
        float sec = dt / 1000.f;
        float tx = bod->x + bod->vx * sec;
        float ty = bod->y + bod->vy * sec;
        bod->vy += g_gravity;

        line_path<> lp{
            static_cast<int>(bod->x), static_cast<int>(bod->y),
            static_cast<int>(tx),  static_cast<int>(ty)};
        
        if (lp.length() == 1) {
            bod->x = tx;
            bod->y = ty;
            return;
        }

        int px, py;

        for (const auto &p : lp) {
            std::tie(px, py) = p;
            if (main_land.is_dirt(px, py)) {
                if (hit_land) {
                    hit_land(bod);
                }
                break;
            }
        }

        bod->x = static_cast<float>(px);
        bod->y = static_cast<float>(py);
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
    std::function<void(std::shared_ptr<body>, std::shared_ptr<body>)> hit_bod;
    std::function<void(std::shared_ptr<body>)> hit_land;
};

}

#endif

