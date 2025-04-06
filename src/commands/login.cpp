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

void IRCServer::user(int fd, std::istringstream &strm_msg)
{
	std::string given_username;
	std::string given_hostname;
	std::string given_servername;
	std::string given_realname;

	strm_msg >> given_username >> given_hostname >> given_servername; 
    std::getline(strm_msg, given_realname);
	if (!given_realname.empty() && given_realname[0] == ' ')
		given_realname = given_realname.substr(1);
	if (given_username.empty() || given_hostname.empty() || given_servername.empty() || given_realname.empty())
		clientLog(fd, "Bad syntax\n");
	else if (!check_realname_syntax(given_realname))
		clientLog(fd, "Bad syntax\n");
	if (!given_realname.empty() && given_realname[0] == ':')
		given_realname = given_realname.substr(1);
	this->_clients[fd]->setUsername(given_username);
	this->_clients[fd]->setHostname(given_hostname);
	this->_clients[fd]->setServername(given_servername);
	this->_clients[fd]->setRealname(given_realname);
	// std::cout << "4. after setting, username " << this->_clients[fd]->getUserData()[0] << std::endl;
	// std::cout << "5. after setting, hostname " << this->_clients[fd]->getUserData()[1] << std::endl;
	// std::cout << "6. after setting, servername " << this->_clients[fd]->getUserData()[2] << std::endl;
	// std::cout << "7. after setting, realname " << this->_clients[fd]->getUserData()[3] << std::endl;
	// the real name can have several words in it.
	// tu dois bien t'assurer que le dernier (realname) commence par un : et peut contenir plusieurs mots 
}