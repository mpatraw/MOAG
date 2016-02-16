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

const float pi = std::acos(-1);

static inline double radians(double degrees) {
    return degrees * (pi / 180.0);
}

static inline double degrees(double radians) {
    return radians * (180.0 / pi);
}

/* Input types.
 * KFIRE_RELEASED is special. It has extra info on the time spent charging up.
 */
enum {
    KLEFT_PRESSED, KLEFT_RELEASED,
    KRIGHT_PRESSED, KRIGHT_RELEASED,
    KUP_PRESSED, KUP_RELEASED,
    KDOWN_PRESSED, KDOWN_RELEASED,
    KFIRE_PRESSED, KFIRE_RELEASED,
};

namespace m {

class land_delegate {
public:
    virtual ~land_delegate() {}
    virtual void set_dirt(int x, int y) = 0;
    virtual void set_air(int x, int y) = 0;
};

class game_controller {
public:
    virtual ~game_controller() {}
    virtual bool solid(int x, int y) = 0;
    virtual bool collide(int x, int y) = 0;
    virtual void explode(int x, int y, int size) = 0;
    virtual void fire(int x, int y, int angle, int velocity) = 0;
};

class land {
public:

    void set_delegate(land_delegate *d) {
        delegate.reset(d);
    }

    void set_dirt(int x, int y) {
        if (x < 0 || y < 0 || x >= g_land_width || y >= g_land_height) {
            return;
        }
        dirt[y * g_land_width + x] = 1;
        if (delegate) {
            delegate->set_dirt(x, y);
        }
    }

    void set_air(int x, int y) {
        if (x < 0 || y < 0 || x >= g_land_width || y >= g_land_height) {
            return;
        }
        dirt[y * g_land_width + x] = 0;
        if (delegate) {
            delegate->set_air(x, y);
        }
    }

    land_delegate const *get_delegate() const { return delegate.get(); }

    bool is_dirt(int x, int y) const {
        if (x < 0 || y < 0 || x >= g_land_width || y >= g_land_height) {
            return true;
        }
        return dirt[y * g_land_width + x] != 0;
    }

    bool is_air(int x, int y) const {
        return !is_dirt(x, y);
    }

    const uint8_t *data() const { return dirt.data(); }

private:
    std::unique_ptr<land_delegate> delegate;
    std::array<uint8_t, g_land_width * g_land_height> dirt;
};

class entity {
public:
    virtual ~entity() { }
};

class body {
public:

    const entity &ent;
    float x, y;
    float vx, vy;
    float w, h;
};

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
    tank tank;

    char name[g_max_name_len];
    bool connected;
    unsigned spawn_timer;
    bool kleft, kright, kup, kdown, kfire;
};

}

/* encoding/decoding
 */
std::vector<uint8_t> rlencode(const std::vector<uint8_t> &src);
std::vector<uint8_t> rldecode(const std::vector<uint8_t> &src);

#endif
