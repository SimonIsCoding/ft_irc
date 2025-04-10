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

#define RPL_JOIN(nick, channel)						(":" + nick + " JOIN " + channel + "\r\n")
#define RPL_PART(client, channel)					(":" + client + " PART " + channel + "\r\n")
#define RPL_MODE(client, channel, mode, name)		(":" + client + " MODE " + channel + " " + mode + " " + name + "\r\n")
#define RPL_KICK(client, channel, target)			(":" + client + " KICK " + channel + " " + target + "\r\n")
#define RPL_INVITE(client, invitee, channel)	(":" + client + " INVITE " + invitee + " " + channel + "\r\n")
#define RPL_NICK(oldNick, newNick)					(":" + oldNick + " NICK " + newNick + "\r\n")
#define RPL_TOPIC(client, channel, topic)			(":" + client + " TOPIC " + channel + " :" + topic + "\r\n")
#define RPL_WELCOME(client)							(": 001 " + client + " :Welcome in the IRC world, " + client + "\r\n")
#define RPL_NOTOPIC(client, channel)				(": 331 " + client + " " + channel + " :No topic is set\r\n")
#define RPL_SEETOPIC(client, channel, topic)		(": 332 " + client + " " + channel + " :" + topic + "\r\n")
#define RPL_INVITESNDR(client, invitee, channel)	(": 341 " + client + " " + invitee + " " + channel + "\r\n")
#define RPL_NAMEREPLY(nick, channel, nicknames)		(": 353 " + nick + " = " + channel + " :" + nicknames + "\r\n")

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
		void ServerCommand();

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
		void bet(int fd, std::istringstream &strm_msg);
		void createChannel(Client *creator, const std::string &name);

		// New HexChat compatible commands
		void part(int fd, std::istringstream &strm_msg);
		void quit(int fd, std::istringstream &strm_msg);
		void whois(int fd, std::istringstream &strm_msg);
		void list(int fd, std::istringstream &strm_msg);
		void names(int fd, std::istringstream &strm_msg);
		void ping(int fd, std::istringstream &strm_msg);
		void pong(int fd, std::istringstream &strm_msg);
		void notice(int fd, std::istringstream &strm_msg);
		void who(int fd, std::istringstream &strm_msg);
		void away(int fd, std::istringstream &strm_msg);

		// fonction interdite:
		void log(int fd, std::istringstream &strm_msg);


		// Dealer func
		void dealerMessage(int fd);
		void createCasino();
	public:
		Server(int port, std::string password);
		~Server();
		void run();

};

#endif