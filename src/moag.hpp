/*
 *
 */
#ifndef MOAG_HPP
#define MOAG_HPP

#include <cmath>
#include <array>

#include "config.hpp"
#include "error.hpp"
#include "network.hpp"
#include "physics.hpp"
#include "util.hpp"

namespace m {

struct tank {
    float x, y;
    float velx, vely;
    int8_t angle;
    int power;
    char bullet;
    int num_burst;
    bool facingleft;
};

class bullet : public entity {
public:
    bullet() :
        body_{std::make_shared<body>(this)},
        physics_body_{std::make_shared<physics_body>(this, body_)} {
        using std::placeholders::_1;
        using std::placeholders::_2;
        physics_body_->hit_land = std::bind(&bullet::hit_land, this, _1, _2);
        physics_body_->hit_body = std::bind(&bullet::hit_body, this, _1);
    }
    virtual ~bullet() {}

    virtual void physics_step(physics &p, float dt) {
        p.step(physics_body_, dt);
    }

    virtual physics_body_action hit_land(float x, float y) {
        (void)x;
        (void)y;
        // Stop falling.
        if (physics_body_->vy < 0) {
            physics_body_->vy = 0;
        }
        alive = false;
        return physics_body_action::stop;
    }

    virtual physics_body_action hit_body(std::shared_ptr<physics_body> &bod) {
        (void)bod;
        return physics_body_action::keep_going;
    }

    body &get_body() { return *body_.get(); }
    const body &get_body() const { return *body_.get(); }
    physics_body &get_physics_body() { return *physics_body_.get(); }
    const physics_body &get_physics_body() const { return *physics_body_.get(); }

    char origin;
    char type;

protected:
    std::shared_ptr<body> body_;
    std::shared_ptr<physics_body> physics_body_;
};

class crate : public entity {
public:
    crate() :
        body_{std::make_shared<body>(this)},
        physics_body_{std::make_shared<physics_body>(this, body_)} {
        using std::placeholders::_1;
        using std::placeholders::_2;
        physics_body_->hit_land = std::bind(&crate::hit_land, this, _1, _2);
        physics_body_->hit_body = std::bind(&crate::hit_body, this, _1);
    }
    virtual ~crate() {}

    virtual void physics_step(physics &p, float dt) {
        p.step(physics_body_, dt);
    }

    virtual physics_body_action hit_land(float x, float y) {
        (void)x;
        (void)y;
        // Stop falling.
        if (physics_body_->vy < 0) {
            physics_body_->vy = 0;
        }
        return physics_body_action::stop;
    }

    virtual physics_body_action hit_body(std::shared_ptr<physics_body> &bod) {
        (void)bod;
        return physics_body_action::keep_going;
    }

    body &get_body() { return *body_.get(); }
    const body &get_body() const { return *body_.get(); }

    char type;

protected:
    std::shared_ptr<body> body_;
    std::shared_ptr<physics_body> physics_body_;
};

struct player {
    tank the_tank;

    char name[g_max_name_len];
    bool connected;
    unsigned spawn_timer;
    bool kleft, kright, kup, kdown, kfire;
};

}

#endif
