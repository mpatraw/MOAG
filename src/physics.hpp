
#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <cassert>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <tuple>

#include "geom.hpp"
#include "land.hpp"
#include "line_path.hpp"

namespace m {

const float cell_size = 40;
// If this is exceeded, some collisions will go unnoticed.
const float max_body_size = cell_size / 2;
const float gravity = 98.f;

enum class body_action : bool {
    stop,
    keep_going
};

class body {
public:
    friend class physics;

    std::shared_ptr<rectangle<>> bounds;
    // If two masks equal something other than 0 when bitwise-anded together,
    // they collide. Default is just 0x00000001.
    int collision_mask;
    float vx;
    float vy;

public:
    body() {}
    body(std::shared_ptr<rectangle<>> bounds, int mask=0x1, float x=0.f, float y=0.f) :
        bounds{bounds}, collision_mask{mask}, vx{x}, vy{y}, steps_til_death_{-1} {}

    void kill() { steps_til_death_ = 1; }
    void unkill() { steps_til_death_ = -1; }
    bool is_dying() const { return steps_til_death_ > 0; }
    bool is_dead() const { return steps_til_death_ == 0; }

    // Both return true when moving should stop.
    std::function<body_action(std::shared_ptr<body>)> hit_body;
    std::function<body_action(int x, int y)> hit_land;

private:
    int steps_til_death_;
};

class physics final {
public:
    physics(const land &l) : main_land_{l} {}

    void step(std::shared_ptr<body> bod, float sec) {
        float tx = bod->bounds->center().x + bod->vx * sec;
        float ty = bod->bounds->center().y + bod->vy * sec;
        // Apply gravity after applying velocity to allow movement when
        // velocity is set manually.
        bod->vy += (gravity * sec);

        line_path<> lp{
            static_cast<int>(bod->bounds->center().x), static_cast<int>(bod->bounds->center().y),
            static_cast<int>(tx),  static_cast<int>(ty)};

        auto neighbors = get_neighbors(bod);

        for (const auto &p : lp) {
            int px, py;
            std::tie(px, py) = p;
            // Check neighbors first.
            bool should_stop = false;
            auto it = std::begin(neighbors);
            while (it != std::end(neighbors)) {
                auto tmp = *bod->bounds;
                tmp.center_on({static_cast<float>(px), static_cast<float>(py)});
                if (tmp.intersects(*(*it)->bounds)) {
                    if (bod->hit_body) {
                        should_stop = bod->hit_body(*it) == body_action::stop || should_stop;
                    }
                    // Don't collide with the same body.
                    it = neighbors.erase(it);
                } else {
                    ++it;
                }
            }
            // For dirt collisions we only care about the bottom middle part
            // of the bounds.
            if (main_land_.is_dirt(px, py + bod->bounds->size.y / 2)) {
                if (bod->hit_land) {
                    should_stop = bod->hit_land(px, py + bod->bounds->size.y / 2) == body_action::stop;
                }
            }

            if (should_stop) {
                break;
            }
            bod->bounds->center_on({static_cast<float>(px), static_cast<float>(py)});
        }
    }

    void step(float dt) {
        auto i = std::begin(body_set_);
        while (i != std::end(body_set_)) {
            if ((*i)->is_dead()) {
                remove_body_from_map(*i);
                i = body_set_.erase(i);
            } else {
                remove_body_from_map(*i);
                step(*i, dt);
                append_body_to_map(*i);
                (*i)->steps_til_death_--;
                ++i;
            }
        }
    }

    // Not safe to call during a step (collision callbacks).
    void append_body(std::shared_ptr<body> bod) {
        assert(bod->bounds->size.x < max_body_size &&
               bod->bounds->size.y < max_body_size && "body too large");
        append_body_to_map(bod);
        body_set_.insert(bod);
    }

    // Not safe to call during a step (collision callbacks).
    void remove_body(std::shared_ptr<body> bod) {
        remove_body_from_map(bod);
        body_set_.erase(bod);
    }

private:
    const land &main_land_;
    std::multimap<std::pair<int, int>, std::shared_ptr<body>> bodies_;
    std::set<std::shared_ptr<body>> body_set_;

    void remove_body_from_map(std::shared_ptr<body> bod) {
        int x = static_cast<int>(bod->bounds->center().x / cell_size);
        int y = static_cast<int>(bod->bounds->center().y / cell_size);
        //printf("removing %p from %d, %d\n", bod.get(), x, y);
        // Now remove it from the multimap.
        auto range = bodies_.equal_range(std::make_pair(x, y));
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second == bod) {
                bodies_.erase(it);
                break;
            }
        }
    }

    void append_body_to_map(std::shared_ptr<body> bod) {
        int x = static_cast<int>(bod->bounds->center().x / cell_size);
        int y = static_cast<int>(bod->bounds->center().y / cell_size);
        //printf("appending %p to %d, %d\n", bod.get(), x, y);
        bodies_.insert(std::make_pair(std::make_pair(x, y), bod));
    }

    std::vector<std::shared_ptr<body>> get_neighbors(std::shared_ptr<body> bod) {
        std::vector<std::shared_ptr<body>> bodies;
        int x = static_cast<int>(bod->bounds->center().x / cell_size);
        int y = static_cast<int>(bod->bounds->center().y / cell_size);
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                auto range = bodies_.equal_range(std::make_pair(x + dx, y + dy));
                for (auto it = range.first; it != range.second; ++it) {
                    // Can't collide with self.
                    if (it->second == bod) {
                        continue;
                    }
                    // Can't collide with a dead body.
                    if (it->second->is_dead()) {
                        continue;
                    }
                    // Do they touch? ;)
                    if (bod->collision_mask & it->second->collision_mask) {
                        bodies.push_back(it->second);
                    }
                }
            }
        }
        return bodies;
    }
};

}

#endif

