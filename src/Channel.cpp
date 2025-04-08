#include "../include/Channel.hpp"

Channel::Channel(std::string name)
	: _name(name), _needpassword(false), _password(""), _admintopic(true), _topic(""), _inviteOnly(true) {
}


Channel::Channel()
	: _name(""), _needpassword(false), _password(""), _admintopic(true), _topic("") {
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
}

const std::string Channel::getChannelName()
{
	return (this->_name);
}

const std::map<int, Client*> Channel::getMembers(){
	return (this->_members);
}

const std::map<int, Client*> Channel::getOperators(){
	return (this->_operators);
}

bool Channel::isOperator(int fd){
	if (_operators.find(fd) == _operators.end())
		return false;
	return true;
}

void	Channel::deleteMember(int fd)
{
	if (_members.find(fd) != _members.end())
		_members.erase(fd);
	if (_operators.find(fd) != _operators.end())
		_operators.erase(fd);
}

void Channel::inviteUser(Client *invited)
{
	std::cout << invited->getNickname() << " a été invité" << std::endl;
	_invited.insert(std::make_pair(invited->getSocket(), invited));
	std::cout << this->isInvited(invited->getSocket()) << std::endl;
}

void Channel::deleteInvitation(int fd){
	std::cout << "1" << std::endl;
	if (_invited.find(fd) != _invited.end())
		_invited.erase(fd);
	std::cout << "2" << std::endl;
}

bool Channel::isInvited(int fd){
	if (_invited.find(fd) == _invited.end())
		return false;
	return true;
}

bool Channel::isMember(int fd)
{
	if (_members.find(fd) != _members.end())
		return true;
	return false;
}