
#if defined(_MSC_VER)
#include <boost/optional.hpp>
template <class T>
using optional = boost::optional<T>;
#else
#include <experimental/optional>
template <class T>
using optional = std::experimental::optional<T>;
#endif
#include <iostream>
#include <string>
#include <thread>

// Defined in server.cpp and client.cpp.
void server_main();
void client_main();

bool g_is_client = true;
bool g_is_server = false;
const char *g_host = "localhost";
int g_port = 6624;

static bool has_arg(int argc, char *argv[], const char *search) {
    for (int i = 0; i < argc; ++i) {
        std::string a{argv[i]};
        if (a.find(search) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Inefficient but gets the work done.
static optional<const char *>
get_arg(int argc, char *argv[], const char *search) {
    for (int i = 0; i < argc; ++i) {
        std::string a{argv[i]};
        if (a.find(search) != std::string::npos) {
            if (i + 1 < argc) {
                return argv[i + 1];
            }
        }
    }
    return {};
}

int main(int argc, char *argv[]) {
    g_is_server = has_arg(argc, argv, "--server");
    g_is_client = !has_arg(argc, argv, "--no-client");
    g_host = get_arg(argc, argv, "--host").value_or("localhost");
    g_port = std::stoi(get_arg(argc, argv, "--port").value_or("6624"));

    std::thread server;
    if (g_is_server) {
        std::cout << "starting server..." << std::endl;
        server = std::thread(server_main);
    }

    if (g_is_client) {
        client_main();
    }

    if (server.joinable()) {
        server.join();
    }
    return 0;
}
