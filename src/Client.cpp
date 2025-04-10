#include "../include/Client.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

Client::Client(int socket) : _socket_fd(socket),  _user_data(4) {
	this->_nickname = "0";
	this->_status = 0;
	this->money = 1001;
	this->_user_data[0] = "username";
	this->_user_data[1] = "hostname";
	this->_user_data[2] = "servername";
	this->_user_data[3] = "realname";
	this->_dcc_active = false;
	this->_dcc_socket = -1;
	this->_dcc_bytes_sent = 0;
	sendMessage("Welcome to the IRC server!\r\n");
}

Client::~Client() {
	cancelDCCTransfer();
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

	return std::string(buffer, valread);
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

bool Client::startDCCSend(const std::string& filename, const std::string& ip, int port, std::streamsize file_size) {
	if (_dcc_active) {
		return false;
	}

	// Create a new socket for DCC transfer
	_dcc_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_dcc_socket < 0) {
		return false;
	}

	// Set socket options
	int opt = 1;
	if (setsockopt(_dcc_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		close(_dcc_socket);
		_dcc_socket = -1;
		return false;
	}

	// Bind to the specified port
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(_dcc_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(_dcc_socket);
		_dcc_socket = -1;
		return false;
	}

	// Listen for incoming connection
	if (listen(_dcc_socket, 1) < 0) {
		close(_dcc_socket);
		_dcc_socket = -1;
		return false;
	}

	// Set socket to non-blocking
	int flags = fcntl(_dcc_socket, F_GETFL, 0);
	fcntl(_dcc_socket, F_SETFL, flags | O_NONBLOCK);

	_dcc_filename = filename;
	_dcc_file_size = file_size;
	_dcc_bytes_sent = 0;
	_dcc_active = true;

	return true;
}

bool Client::startDCCReceive(const std::string& filename, const std::string& ip, int port, std::streamsize file_size) {
	if (_dcc_active) {
		return false;
	}

	_dcc_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_dcc_socket < 0) {
		return false;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(ip.c_str());

	if (connect(_dcc_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
		close(_dcc_socket);
		_dcc_socket = -1;
		return false;
	}

	_dcc_filename = filename;
	_dcc_file_size = file_size;
	_dcc_bytes_sent = 0;
	_dcc_active = true;

	_dcc_file.open(filename.c_str(), std::ios::binary | std::ios::out);
	if (!_dcc_file.is_open()) {
		cancelDCCTransfer();
		return false;
	}

	return true;
}

void Client::handleDCCTransfer() {
	if (!_dcc_active || _dcc_socket < 0) {
		return;
	}

	// Accept incoming connection if not already connected
	if (_dcc_file.is_open()) {
		char buffer[BUFFER_SIZE];
		int bytes_read = recv(_dcc_socket, buffer, BUFFER_SIZE, 0);

		if (bytes_read <= 0) {
			if (errno != EAGAIN && errno != EWOULDBLOCK) {
				cancelDCCTransfer();
			}
			return;
		}

		_dcc_file.write(buffer, bytes_read);
		_dcc_bytes_sent += bytes_read;

		if (_dcc_bytes_sent >= _dcc_file_size) {
			cancelDCCTransfer();
		}
	} else {
		// Accept incoming connection
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_socket = accept(_dcc_socket, (struct sockaddr*)&client_addr, &client_len);

		if (client_socket >= 0) {
			// Close the listening socket
			close(_dcc_socket);
			_dcc_socket = client_socket;

			// Open the file for reading
			std::ifstream file(_dcc_filename.c_str(), std::ios::binary);
			if (!file.is_open()) {
				cancelDCCTransfer();
				return;
			}

			// Send the file
			char buffer[BUFFER_SIZE];
			while (file.read(buffer, BUFFER_SIZE)) {
				int bytes_sent = send(_dcc_socket, buffer, file.gcount(), 0);
				if (bytes_sent <= 0) {
					file.close();
					cancelDCCTransfer();
					return;
				}
				_dcc_bytes_sent += bytes_sent;
			}

			// Send any remaining bytes
			if (file.gcount() > 0) {
				int bytes_sent = send(_dcc_socket, buffer, file.gcount(), 0);
				if (bytes_sent > 0) {
					_dcc_bytes_sent += bytes_sent;
				}
			}

			file.close();
			cancelDCCTransfer();
		} else if (errno != EAGAIN && errno != EWOULDBLOCK) {
			cancelDCCTransfer();
		}
	}
}

void Client::cancelDCCTransfer() {
	if (_dcc_socket >= 0) {
		close(_dcc_socket);
		_dcc_socket = -1;
	}
	if (_dcc_file.is_open()) {
		_dcc_file.close();
	}
	_dcc_active = false;
	_dcc_filename.clear();
	_dcc_file_size = 0;
	_dcc_bytes_sent = 0;
}