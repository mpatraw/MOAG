
#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "../moag.hpp"

namespace m {

class physics final {
public:
    physics(const land &l) : main_land{l} {}

    void step(std::shared_ptr<body> body, uint32_t dt) {
        float sec = dt / 1000.f;
        float tx = body->x + body->vx * sec;
        float ty = body->y + body->vy * sec;
        body->vy += g_gravity;

        line_path<> lp{
            static_cast<int>(body->x), static_cast<int>(body->y),
            static_cast<int>(tx),  static_cast<int>(ty)};
        
        if (lp.length() == 1) {
            body->x = tx;
            body->y = ty;
            return;
        }

        int px, py;

        for (const auto &p : lp) {
            std::tie(px, py) = p;
            if (main_land.is_dirt(px, py)) {
                if (hit_land) {
                    hit_land(body);
                }
                break;
            }
        }

        body->x = static_cast<float>(px);
        body->y = static_cast<float>(py);
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

    void append_body(std::weak_ptr<body> body) {
        bodies.push_back(body);
    }

private:
    const land &main_land;
    std::vector<std::weak_ptr<body>> bodies;
    std::function<void(std::shared_ptr<body>, std::shared_ptr<body>)> hit_body;
    std::function<void(std::shared_ptr<body>)> hit_land;
};

}

#endif

