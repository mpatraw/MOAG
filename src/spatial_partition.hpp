
#ifndef SPATIAL_PARTITION_HPP
#define SPATIAL_PARTITION_HPP

#include <array>
#include <vector>

template <unsigned int W, unsigned int H, unsigned int CS, typename T>
class spatial_partition final {
public:
    spatial_partition() {}

    void add_object(unsigned int x, unsigned int y, T t) {

    }
    void del_object(unsigned int x, unsigned int y, T t) {

    }
    bool does_exist(unsigned int x, unsigned int y, T t) {

    }
private:
    std::array<std::vector<T>, (W * H) / (CS * 2)> grid_;
};

#endif
