#include <cassert>
#include <iostream>

#include "moag.hpp"

int g_port;
const char *g_host;

int main() { 
    m::packet p;

    int16_t sh;
    uint32_t d = htonl(255);

    p.load(static_cast<uint8_t *>(static_cast<void *>(&d)), sizeof(d));
    p << int16_t(15);
    p << "Hello, world";

    p >> d >> sh;

    std::string s;
    p >> s;
    assert(d == 255);
    assert(s == "Hello, world");
    assert(sh == 15);


	int x, y;
	const auto lp = line_path<>(5, 0, 10, 10);
	for (const auto &p : lp) {
		std::tie(x, y) = p;
		std::cout << x << ", " << y << std::endl;
	}
}
