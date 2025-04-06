#include "../../include/Server.hpp"
#include "../../include/Client.hpp"

void IRCServer::pass(int fd, std::istringstream &strm_msg)
{
	std::string given_pass;

	strm_msg >> given_pass;

	if (!checkEmpty(strm_msg) || given_pass.empty())
		clientLog(fd, "Bad syntax\n");
	else if (given_pass == this->_password){
		clientLog(fd, "Good password\n");
		this->_clients[fd]->is_registered = true;
	}else{
		clientLog(fd, "Bad password\n");
	}
}

void IRCServer::nick(int fd, std::istringstream &strm_msg)
{
	std::string given_nick;

	strm_msg >> given_nick;
	if (!checkEmpty(strm_msg) || given_nick.empty())
		clientLog(fd, "Bad syntax\n");

	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (given_nick == it->second->getNickname()) {
			clientLog(fd, "Nickname already used\n");
			return;
		}
	}
	this->_clients[fd]->setNickname(given_nick);
	clientLog(fd, "Nickname has been set\n");
}
