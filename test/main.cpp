
#include <memory>
#include <iostream>

//#include "../src/spatial_partition.hpp"

int main() {
    std::weak_ptr<int> a;
    std::weak_ptr<int> b;
    {
        std::shared_ptr<int> c(new int(5));
        std::cout << (a == c) << std::endl;
        a = c;
    }
    std::cout << (a.lock() == b.lock()) << std::endl;
}
