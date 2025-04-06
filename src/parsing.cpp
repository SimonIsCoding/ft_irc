#include "../include/Server.hpp"
#include "../include/Client.hpp"

void IRCServer::parsing(int client_fd, std::istringstream &strm_msg){
	int i;
	std::string command;
	std::string commands[] = {"PASS", "USER", "NICK"};
	strm_msg >> command;

	for (i = 0 ; i < 3 ; i++){ // 3 is the len of commands[]
		if (commands[i] == command)
			break;
	}
	switch (i)
	{
		case (0):
			pass(client_fd, strm_msg);
			break;
		case (1):
			// clientLog(client_fd, "This command is useless, why is it here anyway ?\n");
			user(client_fd, strm_msg);
			break;
		case (2):
			nick(client_fd, strm_msg);
			break;
	}
}
