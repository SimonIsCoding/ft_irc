#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include "../../include/Channel.hpp"

bool IRCServer::doChannelExist(std::string name){
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->first == name)
			return true;
	}
	return false;
}

void IRCServer::join(int fd, std::istringstream &strm_msg)
{
	std::string channelname;

	strm_msg >> channelname;
	if (!checkEmpty(strm_msg) || channelname.empty())
		return (clientLog(fd, "Bad syntax\n"));
	if (channelname[0] != '#' && channelname[0] != '&')
		return (clientLog(fd, "Channel's name must start with # or &.\n"));
	if (channelname.length() < 2)
		return clientLog(fd, "Channel's name must be at least 2 chars long\n");
	if (doChannelExist(channelname)){
		this->_channels[channelname]->addMember(this->_clients[fd]);
		return (clientLog(fd, "You have joined channel " + channelname + ".\n"));
	}
	else{
		createChannel(_clients[fd], channelname);
		return (clientLog(fd, "You have created and joined channel " + channelname + ".\n"));
	}
}

std::string ltrim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\n\r");
	return (start == std::string::npos) ? "" : str.substr(start);
}

void IRCServer::kick(int fd, std::istringstream &strm_msg){
	std::string channelname;
	std::string nickname;
	std::string reason;
	int dest_fd;

	strm_msg >> channelname >> nickname;
	std::getline(strm_msg, reason);
	reason = ltrim(reason);
	if (!checkEmpty(strm_msg) || nickname.empty() || channelname.empty() || (!reason.empty() && reason[0] != ':'))
		return clientLog(fd, "Bad syntax\n");
	if (!doChannelExist(channelname))
		return clientLog(fd, "Channel does not exist\n");
	dest_fd = getFdByNickname(nickname);
	if (!_channels[channelname]->isOperator(fd))
		return (clientLog(fd, "You are not operator of this channel.\n"));
	if (dest_fd > 0){
		_channels[channelname]->deleteMember(dest_fd);
		if (reason.empty()){
			clientLog(fd, _clients[dest_fd]->getNickname() + " has been kicked from channel " + channelname + ".\n");
			return (clientLog(dest_fd, "You have been kicked from channel " + channelname + ".\n"));
		}
		else{
			clientLog(fd, _clients[dest_fd]->getNickname() + " has been kicked from channel " + channelname + "for reason" + reason + ".\n");
			return (clientLog(dest_fd, "You have been kicked from channel " + channelname + " for reason" + reason + ".\n"));
		}
	}
	else
		clientLog(fd, "User not found.\n");
}