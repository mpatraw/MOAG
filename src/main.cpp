
#include <experimental/optional>
#include <iostream>
#include <string>
#include <thread>

extern "C" void server_main(void);
void client_main();

bool IS_SERVER = false;
const char *HOST = "localhost";
int PORT = 6624;

static bool has_arg(int argc, char *argv[], const char *search)
{
	for (int i = 0; i < argc; ++i) {
		std::string a{argv[i]};
		if (a.find(search) != std::string::npos) {
			return true;
		}
	}
	return false;
}

static std::experimental::optional<const char *>
get_arg(int argc, char *argv[], const char *search)
{
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

int main(int argc, char *argv[])
{
	IS_SERVER = has_arg(argc, argv, "--server");
	HOST = get_arg(argc, argv, "--host").value_or("localhost");
	PORT = std::stoi(get_arg(argc, argv, "--port").value_or("6624"));

	std::thread server;
	if (IS_SERVER) {
		std::cout << "starting server..." << std::endl;
		server = std::thread(server_main);
	}

	client_main();

	if (IS_SERVER) {
		server.join();
	}
	return 0;
}
