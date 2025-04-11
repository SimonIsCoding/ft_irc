#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include "../../include/Channel.hpp"

void Server::invite(int fd, std::istringstream &strm_msg)
{
	std::string channelname;
	std::string nickname;
	std::string reason;
	int dest_fd;

	strm_msg >> nickname  >> channelname;
	if (!checkEmpty(strm_msg) || nickname.empty() || channelname.empty() || (!reason.empty() && reason[0] != ':')) {
		commandLog("INVITE", false);
		return clientLog(fd, "Bad syntax\n");
	}
	if (!doChannelExist(channelname)) {
		commandLog("INVITE", false);
		return clientLog(fd, "Channel does not exist\n");
	}
	dest_fd = getFdByNickname(nickname);
	if (!_channels[channelname]->isOperator(fd)) {
		commandLog("INVITE", false);
		return (clientLog(fd, "You are not operator of this channel.\n"));
	}
	if (dest_fd >  0) {
		if (_channels[channelname]->isMember(dest_fd)) {
			commandLog("INVITE", false);
			return (clientLog(fd, _clients[dest_fd]->getNickname() + " is already in the channel " + channelname + ".\n"));
		}
		if (_channels[channelname]->isInvited(dest_fd)) {
			commandLog("INVITE", false);
			return (clientLog(fd, _clients[dest_fd]->getNickname() + " is already invited to this channel.\n"));
		}
		_channels[channelname]->inviteUser(_clients[dest_fd]);
		clientLog(dest_fd, "You have been invted to join channel " + channelname + ".\n");
		commandLog("INVITE", true);
		return (clientLog(fd, "User " + _clients[dest_fd]->getNickname() + " has been invited to channel " + channelname + ".\n"));
	} else {
		commandLog("INVITE", false);
		return (clientLog(fd, "User not found.\n"));
	}
}