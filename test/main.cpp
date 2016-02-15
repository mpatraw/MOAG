
#include <iostream>

#include "../src/message.hpp"

const char *g_host = "localhost";
int g_port = 6624;

int main() {
    m::message msg(new m::message_message_def{"Hello, world"});
    std::cout << static_cast<int>(msg.get_type()) << std::endl;

    m::serializer s;
    msg.serialize(s);
    s.compress();

    m::serializer d(s.data(), s.length(), s.is_compressed());
    d.decompress();
    m::message msg2(d);
    std::cout << static_cast<int>(msg2.get_type()) << std::endl;
    std::cout << dynamic_cast<m::message_message_def &>(msg2.get_def()).get_string() << std::endl;
}
