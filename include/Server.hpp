#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <stdlib.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <sys/epoll.h>
#include <map>

#define MAX_EVENTS	10
#include "Client.hpp"

class IRCServer {
	private:
		int _server_fd;
		int _port;
		std::string _password;
		struct sockaddr_in address;
		static const int BUFFER_SIZE = 1024;
		std::map<int, Client*> _clients;
		int _epoll_fd;
		void handleNewConnection();
		void handleClientMessage(Client* client);
		void removeClient(Client* client);

	public:
		IRCServer(int port, std::string password);
		~IRCServer();
		void run();
};

#endif