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
#include <cstddef>
#include <sys/epoll.h>
#include <map>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <fcntl.h>
#include <csignal>
#include <cstdlib>


#define MAX_EVENTS	10
#include "Client.hpp"
#include "Channel.hpp"


class Server {
	private:
		int _server_fd;
		int _port;
		std::string _password;
		struct sockaddr_in address;
		static const int BUFFER_SIZE = 1024;
		std::map<int, Client*> _clients;
		std::map<std::string, Channel*> _channels;
		int _epoll_fd;

		void handleNewConnection();
		void handleClientMessage(int client_fd);
		void removeClient(int fd);
		void parsing(int client_fd, std::istringstream &strm_msg);
		bool checkEmpty(std::istringstream &content);
		bool check_realname_syntax(const std::string &content);
		void clientLog(int fd, std::string message);
		bool doChannelExist(std::string name);
		int	getFdByNickname(std::string &nickname);
		void ServerExit();

		void invite(int fd, std::istringstream &strm_msg);
		void pass(int fd, std::istringstream &strm_msg);
		void nick(int fd, std::istringstream &strm_msg);
		void user(int fd, std::istringstream &strm_msg);
		void privmsg(int fd, std::istringstream &message);
		void sendChannel(int fd, std::string &channelname, std::string &content);
		void join(int fd, std::istringstream &strm_msg);
		void kick(int fd, std::istringstream &strm_msg);
		void topic(int fd, std::istringstream &strm_msg);
		void mode(int fd, std::istringstream &strm_msg);
		void topic_mode(int fd, bool addition, std::string channelname);
		void invite_mode(int fd, bool addition, std::string channelname);
		void password_mode(int fd, bool addition, std::string channelname, std::string password);
		void privilege_mode(int fd, bool addition, std::string channelname, std::string privilege);
		void limit_mode(int fd, bool addition, std::string channelname, std::string limit);


		// fonction interdite:
		void log(int fd, std::istringstream &strm_msg);



		void createChannel(Client *creator, const std::string &name);
	public:
		Server(int port, std::string password);
		~Server();
		void run();

};

#endif