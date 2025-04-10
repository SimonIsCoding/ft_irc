#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include <sstream>
#include <arpa/inet.h>

void Server::privmsg(int fd, std::istringstream &message)
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

	// Check for CTCP DCC message
	if (content[0] == '\001' && content[content.length() - 1] == '\001') {
		content = content.substr(1, content.length() - 2); // Remove CTCP delimiters
		std::istringstream dcc_content(content);
		std::string dcc_type;
		dcc_content >> dcc_type;

		if (dcc_type == "DCC") {
			std::string dcc_command;
			std::string filename;
			unsigned long ip_long;
			int port;
			std::streamsize file_size;

			dcc_content >> dcc_command >> filename >> ip_long >> port >> file_size;

			if (dcc_command == "SEND") {
				// Convert IP from long to string
				struct in_addr addr;
				addr.s_addr = htonl(ip_long);
				std::string ip = inet_ntoa(addr);

				// Start receiving the file
				if (_clients[fd]->startDCCReceive(filename, ip, port, file_size)) {
					clientLog(fd, "DCC RECV started successfully.\n");
				} else {
					clientLog(fd, "Failed to start DCC RECV.\n");
				}
				return;
			}
		}
	}

	if (nickname[0] == '#' || nickname[0] == '&')
		return (sendChannel(fd, nickname, content));
	int dest_fd = getFdByNickname(nickname);
	if (dest_fd > 0){
		content = _clients[fd]->getNickname() + ":" + content + '\n';
		send(dest_fd, content.c_str(), content.length(), 0);
		if (dest_fd == 10001) {
			dealerMessage(fd);
		}
	}
	else
		clientLog(fd, "User not found.\n");
}

void Server::sendChannel(int fd, std::string &channelname, std::string &content){
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

void Server::dealerMessage(int fd) {
	std::string message;

	if (_clients[fd]->getMoney() == 0) 
		message = "[Croupier]: I dont talk with poor guy.\n";
	else if (_clients[fd]->getMoney() < 1000) 
		message = "[Croupier]: Haha looks like you loose money on #casino.\n";
	else if(_clients[fd]->getMoney() > 1001)
		message = "[Croupier]: All in for the lore.\n";
	else if(!_channels["#casino"]->isMember(fd))
		message = "[Croupier]: Hello bud, i'm the croupier of the channel #casino. Join us, gambling is funnier than take care of childrens.\n";
	else if(_channels["#casino"]->isMember(fd) && _clients[fd]->getMoney() == 1001) {
		message = "[Croupier]: Looks like you finally discovered the fun part of irc. You can send me a privsmsg if you want to learn how to play.\n";
		_clients[fd]->setMoney(1000);
	}
	else
	{
		message = "[Croupier]: The game is simple. To play, you must use the BET command as following : BET <head or tail> <amount of money>.\n";
		send(fd, message.c_str(), message.length(), 0);
		message = "[Croupier]: You start with 1000$\n";
	}
	send(fd, message.c_str(), message.length(), 0);
}
