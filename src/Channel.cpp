#include "../include/Channel.hpp"

Channel::Channel(std::string name)
	: _name(name), _needpassword(false), _password("") {
}


Channel::Channel()
	: _name(""), _needpassword(false), _password("") {
}

Channel::~Channel() {
	// tout est free dans le destructeur du server normalement

	// for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
	// 	delete (*it).second;
	// }
	// _clients.clear();
}

void Channel::addOperator(Client *op){
	_operators.insert(std::make_pair(op->getSocket(), op));
}

void Channel::addMember(Client *member){
	_members.insert(std::make_pair(member->getSocket(), member));
	std::cout << "User: " << member->getNickname() << " has created channel " << _name << std::endl;
}

const std::string Channel::getChannelName()
{
	return (this->_name);
}
