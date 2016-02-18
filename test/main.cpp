
#include <chrono>
#include <memory>
#include <iostream>
#include <thread>

#include "../src/physics.hpp"

namespace m {

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
