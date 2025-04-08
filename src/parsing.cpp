#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"

void IRCServer::log(int fd, std::istringstream &strm_msg)
{
	_clients[fd]->increaseStatus();
	nick(fd, strm_msg);
	_clients[fd]->increaseStatus();
}

void IRCServer::parsing(int fd, std::istringstream &strm_msg){
	int i;
	std::string command;
	std::string commands[] = {"PASS", "USER", "NICK", "PRIVMSG", "JOIN", "KICK", "TOPIC", "INVITE"};
	strm_msg >> command;

	int len = sizeof(commands) / sizeof(commands[0]);
	if (command == "l")
		return log(fd, strm_msg);
	for (i = 0 ; i < len ; i++){
		if (commands[i] == command)
			break;
	}
	if (i > 2 && _clients[fd]->getStatus() < 3)
		return (clientLog(fd, "You must register first\n"));
	switch (i)
	{
		case (0):
			pass(fd, strm_msg);
			break;
		case (1):
			user(fd, strm_msg);
			break;
		case (2):
			nick(fd, strm_msg);
			break;
		case (3):
			privmsg(fd, strm_msg);
			break;
		case (4):
			join(fd, strm_msg);
			break;
		case (5):
			kick(fd, strm_msg);
			break;
		case (6):
			topic(fd, strm_msg);
			break;
		case (7):
			invite(fd, strm_msg);
			break;
		default:
			clientLog(fd, "Bad input\n");
	}
}

