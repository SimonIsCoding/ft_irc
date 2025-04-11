#include "../include/Server.hpp"

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
		else if (port == 6668) {
			throw std::runtime_error("This port is especially used for the file transfer. Please choose another port between 1 and 65535.");
		}

		if (password.empty()) {
			throw std::runtime_error("Password cannot be empty");
		}

		Server server(port, password);
		server.run();
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
	std::cout << "very gracefully" << std::endl;
	return 0;
}
