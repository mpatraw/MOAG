#include <cassert>

#include "moag.hpp"

int main() { 
    m::packet p;

    uint32_t d = 255;

    p.load(static_cast<uint8_t *>(static_cast<void *>(&d)), sizeof(d));

    assert(p.read32() == 255);
}
