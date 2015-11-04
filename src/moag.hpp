/*
 *
 */
#ifndef MOAG_H
#define MOAG_H

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include <enet/enet.h>

#include "config.hpp"

namespace m {

class client_impl;
class server_impl;

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
        return dirt[y * g_land_width + x];
    }

    bool is_air(int x, int y) const {
        return !is_dirt(x, y);
    }

    const uint8_t *data() const { return dirt.data(); }

private:
    std::unique_ptr<land_delegate> delegate;
    std::array<uint8_t, g_land_width * g_land_height> dirt;
};

class packet final {
public:
    packet() : pos{0} {}

    void reread() { pos = 0; }
    void rewrite() { chunk.clear(); }

    void load(uint8_t *data, size_t len) {
        chunk.clear();
        std::copy_n(data, len, std::back_inserter(chunk));
    }

    void write8(uint8_t i) {
        chunk.push_back(i);
    }
    void write16(uint16_t i) {
        auto v = htons(i);
        chunk.push_back((v >> 8) & 0xff);
        chunk.push_back((v >> 0) & 0xff);
    }
    void write32(uint32_t i) {
        auto v = htonl(i);
        chunk.push_back((v >> 24) & 0xff);
        chunk.push_back((v >> 16) & 0xff);
        chunk.push_back((v >> 8) & 0xff);
        chunk.push_back((v >> 0) & 0xff);
    }

    uint8_t read8() {
        return chunk[pos++];
    }
    uint16_t read16() {
        uint16_t v{0};
        v |= chunk[pos++] << 8;
        v |= chunk[pos++] << 0;
        return ntohs(v);
    }
    uint32_t read32() {
        uint32_t v{0};
        v |= chunk[pos++] << 24;
        v |= chunk[pos++] << 16;
        v |= chunk[pos++] << 8;
        v |= chunk[pos++] << 0;
        return ntohl(v);
    }

private:
    std::vector<uint8_t> chunk;
    size_t pos;
};

class client final {
public:
    client();
    ~client() = default;
    client(const client &) = delete;
    client(client &&) = delete;
    client &operator =(client) = delete;
private:
    std::unique_ptr<client_impl> impl;
};

class server final {
public:
    server();
    ~server() = default;
    server(const server &) = delete;
    server(server &&) = delete;
    server &operator =(server) = delete;
private:
    std::unique_ptr<server_impl> impl;
};

}

/* encoding/decoding
 */
uint8_t *rlencode(const uint8_t *src, size_t len, size_t *outlen);
uint8_t *rldecode(const uint8_t *src, size_t len, size_t *outlen);

#endif
