#include "../include/Client.hpp"

Client::Client(int socket) : _socket_fd(socket),  _user_data(4) {
	this->_nickname = "0";
	this->_status = 0;
	this->money = 1001;
	this->_user_data[0] = "username";
	this->_user_data[1] = "hostname";
	this->_user_data[2] = "servername";
	this->_user_data[3] = "realname";
	if (socket != 10001)
		sendMessage("Welcome to the IRC server!\r\n");
}

Client::~Client() {
	disconnect();
}

void Client::sendMessage(const std::string& message) {
	if (send(_socket_fd, message.c_str(), message.length(), 0) < 0) {
		std::cerr << "Failed to send message to client" << std::endl;
	}
}

std::string Client::receiveMessage() {
	char buffer[BUFFER_SIZE] = {0};
	int valread = read(_socket_fd, buffer, BUFFER_SIZE);

	if (valread <= 0) {
		return "";
	}

	_partial_message.append(buffer, valread);

	return _partial_message;
}

void Client::disconnect() {
	if (_socket_fd != -1) {
		close(_socket_fd);
		_socket_fd = -1;
	}
}

short int	Client::getStatus(void) const{
	return _status;
}

void Client::increaseStatus(void){
	_status++;
}