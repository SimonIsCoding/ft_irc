#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include "../../include/Channel.hpp"

int countMatchingChars(const std::string& s1, bool addition) {
	int count = 0;
	std::string s2;

	if (addition)
		s2 = "kol";
	else
		s2 = "o";

	for (size_t i = 0; i < s1.length(); ++i) {
		char c = s1[i];

		if (s2.find(c) != std::string::npos) {
			++count;
		}
	}
	return count;
}

void	Server::mode(int fd, std::istringstream &strm_msg)
{
	std::string channelname;
	std::string options;
	bool		addition;
	std::string params;
	std::vector<std::string> vec_params;

	std::string mode = "itkol";
	strm_msg >> channelname;
	while (std::getline(strm_msg, params, ' ')) {
		params = ltrim(params);
		if (params[params.length() - 1] == '\n')
			params.erase(params.length() - 1, params.length());
		if (!params.empty())
			vec_params.push_back(params);
	}
	if (vec_params.size() == 0) {
		commandLog("MODE", false);
		return (clientLog(fd, "Bad syntax.\n"));
	}
	options = vec_params[0];
	vec_params.erase(vec_params.begin());
	if (!checkEmpty(strm_msg) || channelname.empty() || options.empty()) {
		commandLog("MODE", false);
		return (clientLog(fd, "Bad syntax.\n"));
	}
	if (!doChannelExist(channelname)) {
		commandLog("MODE", false);
		return (clientLog(fd, "Channel does not exist.\n"));
	}
	if (!_channels[channelname]->isOperator(fd)) {
		commandLog("MODE", false);
		return (clientLog(fd, "Bro who are you ?! You're not even an operator.\n"));
	}
	if (options[0] != '+' && options[0] != '-') {
		commandLog("MODE", false);
		return (clientLog(fd, "First parameter char must be '+' or '-'.\n"));
	}
	if (options[0] == '+')
		addition = true;
	else
		addition = false;
	int char_need_arg = countMatchingChars(options, addition);
	if (char_need_arg != (int)vec_params.size()) {
		commandLog("MODE", false);
		return (clientLog(fd, "Not good number of parameters required.\n"));
	}
	options.erase(0, 1);

	int len = options.length();
	for (int i = 0; i < len; i++)
	{
		int numparam = mode.find(options[i]);
		switch (numparam)
		{
			case (0):
				invite_mode(fd, addition, channelname);
				break;
			case (1):
				topic_mode(fd, addition, channelname);
				break;
			case (2):
				password_mode(fd, addition, channelname, addition ? vec_params[0] : "");
				if (addition)
					vec_params.erase(vec_params.begin());
				break;
			case (3):
				privilege_mode(fd, addition, channelname, vec_params[0]);
				vec_params.erase(vec_params.begin());
				break;
			case (4):
				limit_mode(fd, addition, channelname, addition ? vec_params[0] : "10000");
				if (addition)
					vec_params.erase(vec_params.begin());
				break;
			default:
				commandLog("MODE", false);
				clientLog(fd, "Bad input for MODE\n");
		}
	}
}

void	Server::invite_mode(int fd, bool addition, std::string channelname)
{
	_channels[channelname]->setInviteRights(addition);
	commandLog("MODE", true);
	if (addition)
		return (clientLog(fd, "Invite mode restriction has been set to true.\n"));
	return (clientLog(fd, "Invite mode restriction has been set to false.\n"));
}

void	Server::topic_mode(int fd, bool addition, std::string channelname)
{
	_channels[channelname]->setTopicRights(addition);
	commandLog("MODE", true);
	if (addition)
		return (clientLog(fd, "Topic mode restriction has been set to true.\n"));
	return (clientLog(fd, "Topic mode restriction has been set to false.\n"));
}

void	Server::password_mode(int fd, bool addition, std::string channelname, std::string password)
{
	_channels[channelname]->setPassNeed(addition);
	_channels[channelname]->setPassword(password);
	commandLog("MODE", true);
	if (addition)
		return (clientLog(fd, "Added password successfully.\n"));
	return (clientLog(fd, "Password rule removed.\n"));
}

void	Server::limit_mode(int fd, bool addition, std::string channelname, std::string limit)
{
	int limit_number;
	try{
		limit_number = std::atoi(limit.c_str());
	}
	catch(std::exception& e){
		commandLog("MODE", false);
		return (clientLog(fd, "'" + limit +  "' is not a valid argument for user limit.\n"));
	}
	if (limit_number < 0)
		return (clientLog(fd, "limit given must be a positive number.\n"));
	_channels[channelname]->setUserLimit(limit_number);
	commandLog("MODE", true);
	return (clientLog(fd, "The channel user limit has been set to " + limit + ".\n"));
	(void)addition;
}

void	Server::privilege_mode(int fd, bool addition, std::string channelname, std::string privilege)
{
	int dest_fd = getFdByNickname(privilege);
	if (dest_fd == -1) {
		commandLog("MODE", false);
		return (clientLog(fd, "User not found.\n"));
	}
	if (!_channels[channelname]->isMember(dest_fd)) {
		commandLog("MODE", false);
		return (clientLog(fd, "Bro is not even a member.\n"));
	}
	commandLog("MODE", true);
	if (addition)
	{
		_channels[channelname]->addOperator(_clients[dest_fd]);
		clientLog(dest_fd, "You've been added to operator list of channel: " + channelname + ".\n");
		return (clientLog(fd, "added operator " + privilege + ".\n"));
	}
	else
	{
		_channels[channelname]->rmOperator(dest_fd);
		clientLog(dest_fd, "No one likes you. You've been removed to operator list of channel: " + channelname + ".\n");
		return (clientLog(fd, "removed operator " + privilege + ".\n"));
	}
}