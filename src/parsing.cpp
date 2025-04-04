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
		std::string given_pass;

		strm_msg >> given_pass;

		if (!checkEmpty(strm_msg) || given_pass.empty())
			send(client_fd, "Bad syntax\n", 11, 0);
		else if (given_pass == this->_password){
			send(client_fd, "Good password\n", 14, 0);
			this->_clients[client_fd]->is_registered = true;
		}else{
			send(client_fd, "Bad password\n", 13, 0);
		}
	}
}
