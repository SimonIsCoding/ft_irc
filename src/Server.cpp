#include "../includes/Server.hpp"

int main(int argc, char* argv[]) {
    try {
        int port = 6667;
		if (argc != 3) {
			throw std::runtime_error("Invalid argument number.");
			return (0);
		}

        port = std::atoi(argv[1]);
        std::string password = argv[2];

        if (port <= 0 || port > 65535) {
            throw std::runtime_error("Invalid port number. Must be between 1-65535");
        }

        if (password.empty()) {
            throw std::runtime_error("Password cannot be empty");
        }

        IRCServer server(port);
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
