
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
extern void server_main(unsigned short port);
extern void client_main(const char *host, unsigned short port);

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
            } else {
                std::cout << "warning: no provided argument for " << argv[i] << std::endl;
            }
        }
    }
    return {};
}

int main(int argc, char *argv[]) {
    bool is_server = has_arg(argc, argv, "--server");
    bool is_client = !has_arg(argc, argv, "--no-client");
    const char *host = get_arg(argc, argv, "--host").value_or("localhost");
    unsigned short port = std::stoi(get_arg(argc, argv, "--port").value_or("6624"));

    std::thread server;
    if (is_server) {
        std::cout << "starting server..." << std::endl;
        server = std::thread(server_main, port);
    }

    if (is_client) {
        client_main(host, port);
    }

    if (server.joinable()) {
        server.join();
    }
    return 0;
}
