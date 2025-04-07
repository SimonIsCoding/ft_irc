#include "../../include/Server.hpp"
#include "../../include/Client.hpp"

void IRCServer::join(int fd, std::istringstream &strm_msg)
{
	std::string channelname;

	strm_msg >> channelname;
	if (!checkEmpty(strm_msg) || channelname.empty())
		return (clientLog(fd, "Bad syntax\n"));
	if (channelname[0] != '#' && channelname[0] != '&')
		return (clientLog(fd, "Channel name must start with # or &.\n"));
	createChannel(_clients[fd], channelname);
}