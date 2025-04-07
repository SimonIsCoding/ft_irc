#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include "../include/Channel.hpp"

void IRCServer::parsing(int client_fd, std::istringstream &strm_msg){
	int i;
	std::string command;
	std::string commands[] = {"PASS", "USER", "NICK", "PRIVMSG", "JOIN"};
	strm_msg >> command;

	for (i = 0 ; i < 5 ; i++){ // 3 is the len of commands[]
		if (commands[i] == command)
			break;
	}
	switch (i)
	{
		case (0):
			pass(client_fd, strm_msg);
			break;
		case (1):
			user(client_fd, strm_msg);
			break;
		case (2):
			nick(client_fd, strm_msg);
			break;
		case (3):
			privmsg(client_fd, strm_msg);
			break;
		case (4):
			join(client_fd, strm_msg);
			break;
	}
}
