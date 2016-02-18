
#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <cassert>
#include <cmath>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <tuple>

#include "entity.hpp"
#include "body.hpp"
#include "land.hpp"
#include "line_path.hpp"

namespace m {

const float cell_size = 40;
// If this is exceeded, some collisions will go unnoticed.
const float max_body_size = cell_size / 2;
const float gravity = 98.f;

enum class physics_body_action : uint8_t {
    stop,
    keep_going
};

class physics_body : public component {
public:
    // Must be set.
    std::shared_ptr<body> bod;
    // If collides_with & collision_mask > 0, they collide.
    int collision_mask = 0x1;
    int collides_with = 0x1;
    float vx = 0.f;
    float vy = 0.f;

    std::function<physics_body_action(std::shared_ptr<physics_body> &pb)> hit_body;
    std::function<physics_body_action(int x, int y)> hit_land;

    physics_body(entity *parent, std::shared_ptr<body> bod_) :
        component{parent}, bod{bod_} {
        assert(bod_ && "initialized physics_body without body");
    }
    virtual ~physics_body() {}

    physics_body &with_collision_mask(int collision_mask_) {
        collision_mask = collision_mask_;
        return *this;
    }

    physics_body &with_velocity(float vx_, float vy_) {
        vx = vx_;
        vy = vy_;
        return *this;
    }
};

class physics final {
public:
    physics(const land &l) : main_land_{l} {}

    void step(std::shared_ptr<physics_body> pb, float sec) {
        auto bod = pb->bod;
        // Remove from the grid in-case we move.
        remove_body_from_map(pb);

        float tx = bod->x + pb->vx * sec;
        float ty = bod->y + pb->vy * sec;
        // Apply gravity after applying velocity to allow movement when
        // velocity is set manually.
        pb->vy += (gravity * sec);

        line_path<> lp{
            static_cast<int>(bod->x), static_cast<int>(bod->y),
            static_cast<int>(tx),  static_cast<int>(ty)};

        auto neighbors = get_neighbors(pb);

        for (const auto &p : lp) {
            int px, py;
            std::tie(px, py) = p;
            // Check neighbors first.
            bool should_stop = false;
            auto it = std::begin(neighbors);
            while (it != std::end(neighbors)) {
                auto tmp = *bod;
                tmp.x = static_cast<float>(px);
                tmp.y = static_cast<float>(py);
                auto other_bod = (*it)->bod;
                if (tmp.intersects(*other_bod)) {
                    if (pb->hit_body) {
                        should_stop = pb->hit_body(*it) == physics_body_action::stop || should_stop;
                    }
                    // Don't collide with the same body.
                    it = neighbors.erase(it);
                } else {
                    ++it;
                }
            }
            // For dirt collisions we only care about the bottom middle part
            // of the bod.
            if (main_land_.is_dirt(px, py + bod->height / 2)) {
                if (pb->hit_land) {
                    should_stop = pb->hit_land(px, py + bod->height / 2) == physics_body_action::stop;
                }
            }

            if (should_stop) {
                break;
            }
            bod->x = static_cast<float>(px);
            bod->y = static_cast<float>(py);
        }

        append_body_to_map(pb);
    }

private:
    const land &main_land_;
    std::multimap<std::pair<int, int>, std::weak_ptr<physics_body>> bodies_;

    // Attempts to remove, harmless if the physical_body is not in the list.
    void remove_body_from_map(std::shared_ptr<physics_body> pb) {
        auto bod = pb->bod;
        int x = static_cast<int>(bod->x / cell_size);
        int y = static_cast<int>(bod->y / cell_size);
        //printf("removing %p from %d, %d\n", bod.get(), x, y);
        // Now remove it from the multimap.
        auto range = bodies_.equal_range(std::make_pair(x, y));
        auto it = range.first;
        while (it != range.second) {
            auto check = it->second.lock();
            if (!check) {
                it = bodies_.erase(it);
                continue;
            }
            if (check == pb) {
                bodies_.erase(it);
                break;
            }
            it++;
        }
    }

    void append_body_to_map(std::shared_ptr<physics_body> pb) {
        auto bod = pb->bod;
        int x = static_cast<int>(bod->x / cell_size);
        int y = static_cast<int>(bod->y / cell_size);
        bodies_.insert(std::make_pair(std::make_pair(x, y), pb));
    }

    std::vector<std::shared_ptr<physics_body>> get_neighbors(std::shared_ptr<physics_body> pb) {
        auto bod = pb->bod;
        std::vector<std::shared_ptr<physics_body>> bodies;
        int x = static_cast<int>(bod->x / cell_size);
        int y = static_cast<int>(bod->y / cell_size);
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                auto range = bodies_.equal_range(std::make_pair(x + dx, y + dy));
                auto it = range.first;
                while (it != range.second) {
                    // See if it was free'd.
                    auto other = it->second.lock();
                    if (!other) {
                        // Remove it.
                        it = bodies_.erase(it);
                        continue;
                    }
                    // Can't collide with self.
                    if (other == pb) {
                        continue;
                    }
                    // Do they touch? ;)
                    if (pb->collides_with & other->collision_mask) {
                        bodies.push_back(other);
                    }
                    it++;
                }
            }
        }
        return bodies;
    }
};

}

#endif

