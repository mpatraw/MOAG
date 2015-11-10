/*
 *
 */
#ifndef MOAG_HPP
#define MOAG_HPP

#include <cassert>
#include <cmath>
#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include <enet/enet.h>

#include "config.hpp"
#include "line_path.hpp"
#include "precision_integer.hpp"

static inline double radians(double degrees) {
	const double pi = 3.14159;
	return degrees * (pi / 180.0);
}

static inline double degrees(double radians) {
	const double pi = 3.14159;
	return radians * (180.0 / pi);
}

/******************************************************************************\
Shared structures.
\******************************************************************************/

/* Chunk types. */
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

class network_impl;

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

class tank {
public:
    int x, y;
    int velx, vely;
    int angle, power;
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

class moag {
public:
    player players[g_max_players];
    bullet bullets[g_max_bullets];
    crate crate;
    int frame;
};

const uint8_t packet_type_connection = 0;
const uint8_t packet_type_disconnection = 1;
const uint8_t packet_type_user_defined = 2;

class packet final {
public:
    packet() : pos{0} {}

    bool empty() const { return chunk.empty(); }
    size_t size() const { return chunk.size(); }
    const uint8_t *data() const { return chunk.data(); }
    const uint8_t *remaining_data() const { return data() + pos; }
    size_t remaining_size() const { return size() - pos; }
    bool can_read(size_t amount=1) const { return remaining_size() >= amount; }

    void reread() { pos = 0; }
    void rewrite() { chunk.clear(); }

    void load(uint8_t *data, size_t len) {
        std::copy_n(data, len, std::back_inserter(chunk));
    }

    packet &operator <<(uint8_t i) {
        chunk.push_back(i);
        return *this;
    }
	packet &operator <<(int8_t i) {
		*this << static_cast<uint8_t>(i);
		return *this;
	}
    packet &operator <<(uint16_t i) {
        auto v = htons(i);
        chunk.push_back((v >> 0) & 0xff);
        chunk.push_back((v >> 8) & 0xff);
        return *this;
    }
	packet &operator <<(int16_t i) {
		*this << static_cast<uint16_t>(i);
		return *this;
	}
    packet &operator <<(uint32_t i) {
        auto v = htonl(i);
        chunk.push_back((v >> 0) & 0xff);
        chunk.push_back((v >> 8) & 0xff);
        chunk.push_back((v >> 16) & 0xff);
        chunk.push_back((v >> 24) & 0xff);
        return *this;
    }
	packet &operator <<(int32_t i) {
		*this << static_cast<uint32_t>(i);
		return *this;
	}
    packet &operator <<(const char *str) {
        while (*str) {
            chunk.push_back(*str++);
        }
        chunk.push_back(0);
        return *this;
    }


    packet &operator >>(uint8_t &i) {
        assert(can_read(1));
        i = chunk[pos++];
        return *this;
    }
	packet &operator >>(int8_t &i) {
		*this >> reinterpret_cast<uint8_t &>(i);
		return *this;
	}
    packet &operator >>(uint16_t &i) {
        assert(can_read(2));
        uint16_t v{0};
        v |= chunk[pos++] << 0;
        v |= chunk[pos++] << 8;
        i = ntohs(v);
        return *this;
    }
	packet &operator >>(int16_t &i) {
		*this >> reinterpret_cast<uint16_t &>(i);
		return *this;
	}
    packet &operator >>(uint32_t &i) {
        assert(can_read(4));
        uint32_t v{0};
        v |= chunk[pos++] << 0;
        v |= chunk[pos++] << 8;
        v |= chunk[pos++] << 16;
        v |= chunk[pos++] << 24;
        i = ntohl(v);
        return *this;
    }
	packet &operator >>(int32_t &i) {
		*this >> reinterpret_cast<uint32_t &>(i);
		return *this;
	}
    packet &operator >>(std::string &str) {
        while (auto c = chunk[pos++]) {
            str.push_back(c);
        }
        return *this;
    }

private:
    std::vector<uint8_t> chunk;
    size_t pos;
};

class client final {
public:
    client();
    ~client();
    client(const client &) = delete;
    client(client &&) = delete;
    client &operator =(client) = delete;

    packet &recv();
    void send(const packet &p, bool reliable=true);

    uint32_t rtt() const;
private:
    std::unique_ptr<network_impl> impl;
};

class server final {
public:
    server();
    ~server();
    server(const server &) = delete;
    server(server &&) = delete;
    server &operator =(server) = delete;

    bool is_connected(int id) const;

    packet &recv(int &id);
    void send(const packet &p, int id, bool reliable=true);
    void broadcast(const packet &p, bool reliable=true);

    uint32_t rtt(int id) const;
private:
    std::unique_ptr<network_impl> impl;
};

}

/* encoding/decoding
 */
uint8_t *rlencode(const uint8_t *src, size_t len, size_t *outlen);
uint8_t *rldecode(const uint8_t *src, size_t len, size_t *outlen);

#endif
