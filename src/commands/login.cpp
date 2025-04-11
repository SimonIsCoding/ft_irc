#include "../../include/Server.hpp"
#include "../../include/Client.hpp"

void Server::pass(int fd, std::istringstream &strm_msg)
{
	if (_clients[fd]->getStatus() != 0) {
		commandLog("PASS", true);
		return clientLog(fd, "Password already set\n");
	}
	std::string given_pass;

	strm_msg >> given_pass;

	if (!checkEmpty(strm_msg) || given_pass.empty())
	{
		commandLog("PASS", false);
		return (clientLog(fd, "Bad syntax\n"));
	}
	else if (given_pass == this->_password){
		clientLog(fd, "Good password\n");
		commandLog("PASS", true);
		this->_clients[fd]->increaseStatus();
	}else{
		commandLog("PASS", false);
		clientLog(fd, "Bad password\n");
	}
}

void Server::nick(int fd, std::istringstream &strm_msg)
{
	if (_clients[fd]->getStatus() > 1) {
		commandLog("NICK", true);
		return clientLog(fd, "Nick already set\n");
	}
	else if (_clients[fd]->getStatus() < 1) {
		commandLog("NICK", false);
		return clientLog(fd, "You must type the password first\n");
	}

	std::string given_nick;

	strm_msg >> given_nick;
	if (!checkEmpty(strm_msg) || given_nick.empty())
	{
		commandLog("NICK", false);
		return clientLog(fd, "Bad syntax\n");
	}
	else if (given_nick[0] == '#' || given_nick[0] == '&') {
		commandLog("NICK", false);
		return clientLog(fd, "Your name cannot start with # or &\n");
	}
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (given_nick == it->second->getNickname()) {
			commandLog("NICK", false);
			return clientLog(fd, "Nickname already used\n");
		}
	}
	this->_clients[fd]->setNickname(given_nick);
	this->_clients[fd]->increaseStatus();
	clientLog(fd, "Nickname has been set\n");
	commandLog("NICK", true);
}

void Server::user(int fd, std::istringstream &strm_msg)
{
	if (_clients[fd]->getStatus() == 0) {
		commandLog("USER", false);
		return clientLog(fd, "You must type the password first\n");
	}
	else if (_clients[fd]->getStatus() == 1) {
		commandLog("USER", false);
		return clientLog(fd, "You must set your nickname first\n");
	}
	else if (_clients[fd]->getStatus() > 2) {
		commandLog("USER", true);
		return clientLog(fd, "You already are registered\n");
	}

	std::string given_username;
	std::string given_hostname;
	std::string given_servername;
	std::string given_realname;

	strm_msg >> given_username >> given_hostname >> given_servername;
	std::getline(strm_msg, given_realname);
	if (!given_realname.empty() && given_realname[0] == ' ')
		given_realname = given_realname.substr(1);
	if (given_username.empty() || given_hostname.empty() || given_servername.empty() || given_realname.empty()) {
		commandLog("USER", false);
		return (clientLog(fd, "Bad syntax\n"));
	}
	else if (!check_realname_syntax(given_realname)) {
		commandLog("USER", false);
		return (clientLog(fd, "Bad syntax\n"));
	}
	if (!given_realname.empty() && given_realname[0] == ':')
		given_realname = given_realname.substr(1);
	this->_clients[fd]->setUsername(given_username);
	this->_clients[fd]->setHostname(given_hostname);
	this->_clients[fd]->setServername(given_servername);
	this->_clients[fd]->setRealname(given_realname);
	this->_clients[fd]->increaseStatus();
	clientLog(fd, "User has been set\n");
	commandLog("USER", true);
}