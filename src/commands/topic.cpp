#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include "../../include/Channel.hpp"

void Server::topic(int fd, std::istringstream &strm_msg){
	std::string channelname;
	std::string newtopic;

	strm_msg >> channelname;
	std::getline(strm_msg, newtopic);
	newtopic = ltrim(newtopic);
	if (!checkEmpty(strm_msg) || channelname.empty() || (!newtopic.empty() && newtopic[0] != ':' && newtopic.length() > 2)) {
		commandLog("TOPIC", false);
		return (clientLog(fd, "Bad syntax.\n"));
	}
	newtopic.erase(0, 1);
	if (!doChannelExist(channelname)) {
		commandLog("TOPIC", false);
		return clientLog(fd, "Channel does not exist\n");
	}
	if (newtopic.empty()){
		if (_channels[channelname]->getTopic().empty()) {
			commandLog("TOPIC", true);
			return (clientLog(fd, "No topic set yet for channel " + channelname + ".\n"));
		}
		commandLog("TOPIC", true);
		return (clientLog(fd, channelname + "'s topic: " + _channels[channelname]->getTopic()+ ".\n"));
	}
	if ((_channels[channelname]->isOperator(fd) && _channels[channelname]->getTopicRights()) || !_channels[channelname]->getTopicRights()){
		_channels[channelname]->setTopic(newtopic);
		commandLog("TOPIC", true);
		return (clientLog(fd, channelname + "'s topic set to: " + _channels[channelname]->getTopic()+ ".\n"));
	}
	else {
		commandLog("TOPIC", false);
		return (clientLog(fd, "You are not operator so you can't change topic.\n"));
	}
}