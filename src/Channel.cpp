#include "../include/Channel.hpp"

Channel::Channel(std::string name)
	: _name(name), _needpassword(false), _password(""), _admintopic(false), _topic(""), _inviteOnly(false), _limit(10000) {
}


Channel::Channel()
	: _name(""), _needpassword(false), _password(""), _admintopic(false), _topic(""), _inviteOnly(false), _limit(10000) {
}

Channel::~Channel() {
}

void Channel::addOperator(Client *op){
	_operators.insert(std::make_pair(op->getSocket(), op));
}

void Channel::rmOperator(int fd){
	if (_members.find(fd) != _members.end())
		_operators.erase(fd);
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
	_invited.insert(std::make_pair(invited->getSocket(), invited));
}

void Channel::deleteInvitation(int fd){
	if (_invited.find(fd) != _invited.end())
		_invited.erase(fd);
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