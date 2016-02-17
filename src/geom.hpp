
#ifndef GEOM_HPP
#define GEOM_HPP

namespace m {

template <typename T=float>
struct point final {
    T x, y;

    const T &operator [](size_t i) const { return i == 0 ? x : y; }
    T &operator [](size_t i) { return i == 0 ? x : y; }
};

template <typename T=float>
struct rectangle final {
    point<T> pos;
    point<T> size;

    rectangle() {}
    rectangle(const point<T> &pos, const point<T> &size) : pos{pos}, size{size} {}
    rectangle(const T &x, const T &y, const T &w, const T &h) : rectangle{{x, y}, {w, h}} {}

    point<T> center() const { return {pos.x + size.x / 2, pos.y + size.y / 2}; }
    void center_on(const point<T> p) { pos.x = p.x - size.x / 2; pos.y = p.y - size.y / 2; }

    bool intersects(const rectangle &other) const {
        auto c = center();
        auto other_c = other.center();
        bool a = std::abs(c.x - other_c.x) * 2 > size.x + other.size.x;
        bool b = std::abs(c.y - other_c.y) * 2 > size.x + other.size.y;
        return !a && !b;
    }
};

}

#endif
