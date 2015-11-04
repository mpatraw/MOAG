#include <cassert>

#include "moag.hpp"

int g_port;
const char *g_host;

int main() { 
    m::packet p;

    uint32_t d = 255;

    p.load(static_cast<uint8_t *>(static_cast<void *>(&d)), sizeof(d));
    p << "Hello, world";

    p >> d;

    std::string s;
    p >> s;
    assert(d == 255);
    assert(s == "Hello, world");
}
