#include "../include/Client.hpp"

Client::Client(int socket) : socket_fd(socket), is_registered(false) {
	this->_nickname = "0";
	sendMessage("Welcome to the IRC server!\r\n");
}

Client::~Client() {
	disconnect();
}

void Client::sendMessage(const std::string& message) {
	if (send(socket_fd, message.c_str(), message.length(), 0) < 0) {
		std::cerr << "Failed to send message to client" << std::endl;
	}
}

std::string Client::receiveMessage() {
	char buffer[BUFFER_SIZE] = {0};
	int valread = read(socket_fd, buffer, BUFFER_SIZE);

	if (valread <= 0) {
		return "";
	}

	return std::string(buffer, valread);
}

void Client::disconnect() {
	if (socket_fd != -1) {
		close(socket_fd);
		socket_fd = -1;
	}
}