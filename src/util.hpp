
#ifndef UTIL_HPP
#define UTIL_HPP

const float pi = std::acos(-1);

static inline double radians(double degrees) {
    return degrees * (pi / 180.0);
}

static inline double degrees(double radians) {
    return radians * (180.0 / pi);
}

#endif
