#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include "Client.hpp"

std::string ltrim(const std::string& str);

class Channel {
	private:
		std::string _name;

		bool		_needpassword;
		std::string	_password;
		bool		_admintopic;
		std::string _topic;
		bool		_inviteOnly;

		std::map<int, Client*> _members;
		std::map<int, Client*> _operators;
		std::map<int, Client*> _invited;


	public:
		void addOperator(Client *op);
		void addMember(Client *member);
		const std::map<int, Client*> getMembers();
		const std::map<int, Client*> getOperators();
		const std::string getChannelName();
		bool isOperator(int fd);
		bool isMember(int fd);
		bool isInvited(int fd);
		void deleteInvitation(int fd);
		void deleteMember(int fd);
		std::string getTopic(void) {return _topic;};
		void setTopic(const std::string &topic) {_topic = topic;};

		bool getTopicRights(void) {return _admintopic;};
		void setTopicRights(bool rights) {_admintopic = rights;};

		bool getInviteRights(void) {return _inviteOnly;};
		void setInviteRights(bool rights) {_inviteOnly = rights;};

		void inviteUser(Client *invited);

		Channel();
		Channel(std::string name);
		~Channel();
};

#endif