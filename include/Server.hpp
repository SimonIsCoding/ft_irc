#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "Client.hpp"

class IRCServer {
	private:
		int server_fd;
		int port;
		struct sockaddr_in address;
		static const int BUFFER_SIZE = 1024;
		std::vector<Client*> clients;
		fd_set master_fds;
		int max_fd;

		void handleClient(int client_socket);
		void handleNewConnection();
		void handleClientMessage(Client* client);
		void removeClient(Client* client);

	public:
		IRCServer(int port);
		~IRCServer();
		void run();
};

#endif 