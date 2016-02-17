/*
 *
 */
#ifndef MOAG_HPP
#define MOAG_HPP

#include <cmath>
#include <array>

#include "config.hpp"
#include "error.hpp"
#include "line_path.hpp"
#include "network.hpp"
#include "physics.hpp"
#include "util.hpp"

namespace m {

class tank {
public:
    float x, y;
    float velx, vely;
    int8_t angle;
    int power;
    char bullet;
    int num_burst;
    bool facingleft;
};

class bullet {
public:
    float x, y;
    float velx, vely;
    int origin;
    char active;
    char type;
};

class crate {
public:
    float x, y;
    float velx, vely;
    bool active;
    char type;
};

class player {
public:
    tank the_tank;

    char name[g_max_name_len];
    bool connected;
    unsigned spawn_timer;
    bool kleft, kright, kup, kdown, kfire;
};

}

#endif
