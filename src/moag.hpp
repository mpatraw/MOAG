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

struct bullet {
    float x, y;
    float velx, vely;
    int origin;
    char active;
    char type;
};

struct crate {
    float x, y;
    float velx, vely;
    bool active;
    char type;
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
