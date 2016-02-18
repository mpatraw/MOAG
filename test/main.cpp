
#include <chrono>
#include <memory>
#include <iostream>
#include <thread>

#include "../src/physics.hpp"

namespace m {

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
        printf("%f, %f\n", body_->x, body_->y);
        p.step(physics_body_, dt);
    }

    virtual physics_body_action hit_land(float x, float y) {
        (void)x;
        (void)y;
        printf("hit land at: %f, %f\n", x, y);
        return physics_body_action::stop;
    }

    virtual physics_body_action hit_body(std::shared_ptr<physics_body> &bod) {
        (void)bod;
        printf("hit body %p\n", bod.get());
        return physics_body_action::keep_going;
    }

protected:
    std::shared_ptr<body> body_;
    std::shared_ptr<physics_body> physics_body_;
};

}

int main() {
    m::land land;
    m::physics physics{land};
    auto crate = m::crate{};
    auto crate1 = m::crate{};
    while (true) {
        crate.physics_step(physics, .1f);
        crate1.physics_step(physics, .1f);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
