#include "../../include/Server.hpp"
#include "../../include/Client.hpp"

void IRCServer::privmsg(int fd, std::istringstream &message)
{
	std::string user;
	std::string content;

	message >> user;
	std::getline(message, content);
	if (user.empty() || content.empty())
	{
		clientLog(fd, "Bad use of command privmsg.\n");
		return;
	}
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == user){
			content = _clients[fd]->getNickname() + ":" + content + '\n';
			send(it->first, content.c_str(), content.length(), 0);
			return;
		}
	}
	clientLog(fd, "user not found");
}
