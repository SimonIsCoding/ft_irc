#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <fstream>

class Client {
	private:
		short int _status;
		int _socket_fd;
		std::string _nickname;
		std::string _username;
		std::vector<std::string> _user_data;
		int money;

		// DCC transfer related members
		int _dcc_socket;
		std::string _dcc_filename;
		std::streamsize _dcc_file_size;
		std::streamsize _dcc_bytes_sent;
		std::ofstream _dcc_file;
		bool _dcc_active;

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

		// DCC transfer related methods
		bool startDCCSend(const std::string& filename, const std::string& ip, int port, std::streamsize file_size);
		bool startDCCReceive(const std::string& filename, const std::string& ip, int port, std::streamsize file_size);
		void handleDCCTransfer();
		void cancelDCCTransfer();
		bool isDCCActive() const { return _dcc_active; }

		void sendMessage(const std::string& message);
		std::string receiveMessage();
		void disconnect();
		short int getStatus() const;
		void increaseStatus();
};

#endif