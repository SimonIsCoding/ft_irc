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

class Channel {
	private:
		std::string _name;

		bool		_needpassword;
		std::string	_password;

		std::map<int, Client*> _members;
		std::map<int, Client*> _operators;


	public:
		void addOperator(Client *op);
		void addMember(Client *member);
		const std::string getChannelName();

		Channel();
		Channel(std::string name);
		~Channel();
};

#endif