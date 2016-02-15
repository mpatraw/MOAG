
#include <iostream>

#include "../src/serialize.hpp"

const char *g_host = "localhost";
int g_port = 6624;

static auto i = 100;
static auto b = true;
static auto f = 3.14;

static void serialize(m::serializer &s) {
    s.decompress();
    s & i;
    s & b;
    s & f;
    s.compress();
}

int main() {
    m::serializer s;
    serialize(s);

    m::serializer d(s.data(), s.length());
    serialize(d);
    
    std::cout << i << " " << b << " " << f << std::endl;
}
