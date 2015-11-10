/*
 *
 */
#ifndef MOAG_HPP
#define MOAG_HPP

#include <array>

#include "config.hpp"
#include "line_path.hpp"
#include "network.hpp"
#include "precision_integer.hpp"

static inline double radians(double degrees) {
	const double pi = 3.14159;
	return degrees * (pi / 180.0);
}

static inline double degrees(double radians) {
	const double pi = 3.14159;
	return radians * (180.0 / pi);
}

enum {
    /******************
     * Client -> Server
     */

    /* RELIABLE
     * 1: INPUT_CHUNK
     * 1: *_*_CHUNK
     * 2: Milliseconds held.
     */
    INPUT_CHUNK = 2,

    /* RELIABLE
     * 1: CLIENT_MSG_CHUNK
     * length: characters
     */
    CLIENT_MSG_CHUNK,

    /******************
     * Server -> Client
     */

    /* RELIABLE
     * 1: LAND_CHUNK
     * 2: x-position
     * 2: y-position
     * 2: width
     * 2: height
     * X: zipped land (columns first)
     */
    LAND_CHUNK,
    /* RELIABLE
     * 1: PACKED_LAND_CHUNK
     * 2: x-position
     * 2: y-position
     * 2: width
     * 2: height
     * 4: packed-size
     * X: RLE-compressed data
     */
    PACKED_LAND_CHUNK,
    /* VARIES
     * 1: TANK_CHUNK
     * 1: SPAWN/KILL/MOVE
     * 1: id
     * IF NOT KILL
     *  2: x-position
     *  2: y-position
     *  1: angle (0 to 180), 0 is left, 180 is right
     */
    TANK_CHUNK,
    /* VARIES
     * 1: BULLET_CHUNK
     * 1: SPAWN/KILL/MOVE
     * 1: id
     * IF NOT KILL
     *  2: x-position
     *  2: y-position
     */
    BULLET_CHUNK,
    /* VARIES
     * 1: CREATE_CHUNK
     * 1: SPAWN/KILL/MOVE
     * IF NOT KILL
     *  2: x-position
     *  2: y-position
     */
    CRATE_CHUNK,
    /* RELIABLE
     * 1: SERVER_MSG_CHUNK
     * 1: id
     * 1: CHAT/NAME_CHANGE/SERVER_NOTICE
     * length: characters
     */
    SERVER_MSG_CHUNK,
};

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

enum {
    /* RELIABLE */
    SPAWN,
    /* RELIABLE */
    KILL,
    /* UNRELIABLE */
    MOVE,
};

/* SERVER_MSG_CHUNK commands */
enum {
    CHAT,
    SERVER_NOTICE,
    NAME_CHANGE,
};

namespace m {

class initialization_error {
public:
    initialization_error(const char *msg) : whatstr{msg} { }

    virtual const char *what() const { return whatstr.c_str(); }
protected:
    std::string whatstr;
};

class land_delegate {
public:
    virtual ~land_delegate() {}
    virtual void set_dirt(int x, int y) = 0;
    virtual void set_air(int x, int y) = 0;
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

class tank final {
public:
    int16_t x, y;
	int8_t angle;
    int velx, vely;
    int power;
    char bullet;
    int num_burst;
    bool facingleft;
};

class bullet {
public:
    int x, y;
    int velx, vely;
	int origin;
    char active;
    char type;
};

class crate {
public:
    int x, y;
    bool active;
    char type;
};

class player {
public:
    tank tank;

    char name[g_max_name_len];
    bool connected;
    unsigned spawn_timer;
    unsigned ladder_count;
    int ladder_timer;
    bool kleft, kright, kup, kdown, kfire;
};

}

/* encoding/decoding
 */
uint8_t *rlencode(const uint8_t *src, size_t len, size_t *outlen);
uint8_t *rldecode(const uint8_t *src, size_t len, size_t *outlen);

#endif
