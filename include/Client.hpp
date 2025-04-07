#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <vector>

class Client {
	private:
		int _socket_fd;
		std::string _nickname;
		std::string _username;
		std::vector<std::string> _user_data;

		static const int BUFFER_SIZE = 1024;

	public:
		bool is_registered;
		Client(int socket);
		~Client();

		int getSocket() const { return _socket_fd; }
		std::string getNickname() const { return _nickname; }
		void setNickname(const std::string nickname) { this->_nickname = nickname; }
		bool isRegistered() const { return is_registered; }
		const std::vector<std::string>& getUserData() const { return _user_data; }
		void setUsername(const std::string username) { this->_user_data[0] = username; }
		void setHostname(const std::string hostname) { this->_user_data[1] = hostname; }
		void setServername(const std::string servername) { this->_user_data[2] = servername; }
		void setRealname(const std::string realname) { this->_user_data[3] = realname; }

		void sendMessage(const std::string& message);

		std::string receiveMessage();
		void disconnect();
};

#endif