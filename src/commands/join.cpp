#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include "../../include/Channel.hpp"

bool Server::doChannelExist(std::string name){
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->first == name)
			return true;
	}
	return false;
}

void Server::join(int fd, std::istringstream &strm_msg)
{
	std::string channelname;
	std::string password;

	strm_msg >> channelname;
	if (doChannelExist(channelname) && _channels[channelname]->getPassNeed())
	{
		strm_msg >> password;
		if (password.empty())
			return (clientLog(fd, "A password is needed to get inside this channel.\n"));
		if (_channels[channelname]->getPassword() != password)
			return (clientLog(fd, "Bad password.\n"));
	}
	if (!checkEmpty(strm_msg) || channelname.empty())
		return (clientLog(fd, "Bad syntax\n"));
	if (channelname[0] != '#' && channelname[0] != '&')
		return (clientLog(fd, "Channel's name must start with # or &.\n"));
	if (channelname.length() < 2)
		return clientLog(fd, "Channel's name must be at least 2 chars long\n");
	if (doChannelExist(channelname))
	{
		if (_channels[channelname]->isMember(fd))
			return (clientLog(fd, "You already are part of this channel.\n"));
		if (_channels[channelname]->getUserLimit() <= (int)_channels[channelname]->getMembers().size())
			return (clientLog(fd, "User limit for this channel has been reached.\n"));
		if ((_channels[channelname]->getInviteRights() && _channels[channelname]->isInvited(fd)) || !_channels[channelname]->getInviteRights()){
			this->_channels[channelname]->addMember(this->_clients[fd]);
			if (_channels[channelname]->getInviteRights())
				_channels[channelname]->deleteInvitation(fd);
			if (channelname == "#casino") {
				return(dealerMessage(fd));
			}
			return (clientLog(fd, "You have joined channel " + channelname + ".\n"));
		}
		return (clientLog(fd, "You are not invited to join this channel.\n"));
	}
	else
	{
		createChannel(_clients[fd], channelname);
		return (clientLog(fd, "You have created and joined channel " + channelname + ".\n"));
	}
}

std::string ltrim(const std::string& str) {
	size_t start = str.find_first_not_of(" \t\n\r");
	return (start == std::string::npos) ? "" : str.substr(start);
}

void Server::kick(int fd, std::istringstream &strm_msg){
	std::string channelname;
	std::string nickname;
	std::string reason;
	int dest_fd;

	strm_msg >> channelname >> nickname;
	std::getline(strm_msg, reason);
	reason = ltrim(reason);
	if (!checkEmpty(strm_msg) || nickname.empty() || channelname.empty() || (!reason.empty() && reason[0] != ':'))
		return clientLog(fd, "Bad syntax\n");
	reason.erase(0, 1);
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
			clientLog(fd, _clients[dest_fd]->getNickname() + " has been kicked from channel " + channelname + " for reason: " + reason + ".\n");
			return (clientLog(dest_fd, "You have been kicked from channel " + channelname + " for reason: " + reason + ".\n"));
		}
	}
	else
		clientLog(fd, "User not found.\n");
}