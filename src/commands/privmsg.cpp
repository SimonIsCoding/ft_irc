#include "../../include/Server.hpp"
#include "../../include/Client.hpp"

void IRCServer::privmsg(int fd, std::istringstream &message)
{
	std::string nickname;
	std::string content;

	message >> nickname;
	std::getline(message, content);
	if (nickname.empty() || content.empty())
	{
		clientLog(fd, "Bad use of command privmsg.\n");
		return;
	}
	if (nickname[0] == '#' || nickname[0] == '&')
		return (sendChannel(fd, nickname, content));
	int dest_fd = getFdByNickname(nickname);
	if (dest_fd > 0){
		content = _clients[fd]->getNickname() + ":" + content + '\n';
		send(dest_fd, content.c_str(), content.length(), 0);
	}
	else
		clientLog(fd, "User not found.\n");
}

void IRCServer::sendChannel(int fd, std::string &channelname, std::string &content){
	if (!doChannelExist(channelname))
		return clientLog(fd, "Channel do not exist.\n");
	std::map<int, Client*> members = _channels[channelname]->getMembers();
	content = "[" + channelname + "] " + _clients[fd]->getNickname() + ":" + content + '\n';
	if (members.find(fd) == members.end())
		return clientLog(fd, "You are not member of this channel.\n");
	for (std::map<int, Client*>::iterator it = members.begin(); it != members.end(); ++it) {
		if (it->first != fd){
			send(it->first, content.c_str(), content.length(), 0);
		}
	}
}
