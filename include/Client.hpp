#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

class Client {
	private:
		int socket_fd;
		std::string nickname;
		std::string username;

		static const int BUFFER_SIZE = 1024;

		public:
		bool is_registered;
		Client(int socket);
		~Client();

		int getSocket() const { return socket_fd; }
		std::string getNickname() const { return nickname; }
		bool isRegistered() const { return is_registered; }

		void sendMessage(const std::string& message);
		std::string receiveMessage();
		void disconnect();
};

#endif