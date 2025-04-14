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
		short int _status;
		int _socket_fd;
		std::string _nickname;
		std::string _username;
		std::vector<std::string> _user_data;
		int money;
		std::string _partial_message;

		static const int BUFFER_SIZE = 1024;

	public:
		Client(int socket);
		~Client();

		int getSocket() const { return _socket_fd; }
		std::string getNickname() const { return _nickname; }
		void setNickname(const std::string nickname) { this->_nickname = nickname; }
		const std::vector<std::string>& getUserData() const { return _user_data; }
		void setUsername(const std::string username) { this->_user_data[0] = username; }
		void setHostname(const std::string hostname) { this->_user_data[1] = hostname; }
		void setServername(const std::string servername) { this->_user_data[2] = servername; }
		void setRealname(const std::string realname) { this->_user_data[3] = realname; }
		int getMoney(void) const { return this->money; }
		void setMoney(int newAmount) {this->money = newAmount >= 0 ? newAmount : 0; }
		void setPartial(std::string newMessage) { _partial_message = newMessage; }

		void sendMessage(const std::string& message);
		std::string receiveMessage();
		void disconnect();
		short int getStatus() const;
		void increaseStatus();
};

#endif