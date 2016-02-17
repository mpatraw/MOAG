
#include <chrono>
#include <memory>
#include <iostream>
#include <thread>

#include "../src/physics.hpp"

int main() {
    bool running = true;
    m::land land{};
    m::physics physics{land};
    auto bounds0 = std::make_shared<m::rectangle<>>(0.f, 0.f, 10.f, 10.f);
    auto body0 = std::make_shared<m::body>(bounds0);
    auto bounds1 = std::make_shared<m::rectangle<>>(0.f, 0.f, 10.f, 10.f);
    auto body1 = std::make_shared<m::body>(bounds1);
    body0->hit_land = [&](int x, int y) {
        std::cout << "hit land at: " << x << ", " << y << std::endl;
        body0->vy = 0;
        running = false;
        return m::body_action::stop;
    };
    body0->hit_body = [&](std::shared_ptr<m::body> other) {
        other->kill();
        std::cout << "hit body 1 and killed" << std::endl;
        return m::body_action::keep_going;
    };
    body1->hit_body = [&](std::shared_ptr<m::body> other) {
        (void)other;
        std::cout << "hit body 0" << std::endl;
        return m::body_action::keep_going;
    };
    physics.append_body(body0);
    physics.append_body(body1);
    while (running) {

        physics.step(0.1);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
