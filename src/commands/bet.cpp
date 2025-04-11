#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include "../../include/Channel.hpp"

bool isNumber(const std::string& s) {
    std::istringstream iss(s);
    double d;
    char c;

    if (!(iss >> d)) return false;

    if (iss >> c) return false;

    return true;
}


void Server::bet(int fd, std::istringstream &message)
{
	std::string side;
	std::string amount;
	int moneyAmount;
	std::string answerMessage;

	message >> side >> amount;
	if (side.empty() || amount.empty())
	{
		clientLog(fd, "Bad use of command bet.\n");
		return;
	}
	if (!_channels["#casino"]->isMember(fd)) {
		return (clientLog(fd, "You must join #casino before.\n"));
	}

	try{
		moneyAmount = std::atoi(amount.c_str());
	}
	catch(std::exception& e){
		return (clientLog(fd, "'" + amount +  "' is not a valid argument for bet.\n"));
	}

	if (isNumber(side))
	{
		answerMessage = "[Croupier]: Did you really write the amount instead of the side of the coin ??\n";
		send(fd, answerMessage.c_str(), answerMessage.length(), 0);
		return ;
	}

	if (moneyAmount > _clients[fd]->getMoney()) {
		answerMessage = "[Croupier]: Well, well, well.\n";
		send(fd, answerMessage.c_str(), answerMessage.length(), 0);
		answerMessage =  "[Croupier]: YOU DONT HAVE ENOUGH MONEY POOR GUY.\n";
		send(fd, answerMessage.c_str(), answerMessage.length(), 0);
		return ;
	}

	if (side != "head" && side != "tail" && side != "both") 
	{
		answerMessage = "[Croupier]: Does " + side + " looks like a valid option to you ?!\n";
		send(fd, answerMessage.c_str(), answerMessage.length(), 0);
		return ;
	}

	int random = rand() % 101;
	std::cout << random << std::endl;
	int clientMoney = _clients[fd]->getMoney();
	_clients[fd]->setMoney(clientMoney - moneyAmount);
	if (side == "tail" && random < 51) {
		_clients[fd]->setMoney(clientMoney + moneyAmount);
		std::stringstream ss;
		ss << "You now have " << _clients[fd]->getMoney() << "$\n";
		clientLog(fd, ss.str());
		answerMessage = "[Croupier]: You win for this round, try again and all in for the lore.\n";
		send(fd, answerMessage.c_str(), answerMessage.length(), 0);
		return ;
	}
	else if (side == "head" && random > 50) {
		_clients[fd]->setMoney(clientMoney + moneyAmount);
		std::stringstream ss;
		ss << "You now have " << _clients[fd]->getMoney() << "$\n";
		clientLog(fd, ss.str());
		answerMessage = "[Croupier]: You win for this round, try again and all in for the lore.\n";
		send(fd, answerMessage.c_str(), answerMessage.length(), 0);
		return ;
	}
	else if (side == "both") {
		_clients[fd]->setMoney(1000000000);
		std::stringstream ss;
		ss << "You now have " << _clients[fd]->getMoney() << "$\n";
		clientLog(fd, ss.str());
		answerMessage = "[Croupier]: You win for this round, try again and all in for the lore.\n";
		send(fd, answerMessage.c_str(), answerMessage.length(), 0);
		return ;
	}
	else{
		std::stringstream ss;
		ss << "You now have " << _clients[fd]->getMoney() << "$\n";
		clientLog(fd, ss.str());
		if (_clients[fd]->getMoney() == 0)
		{
			answerMessage = "[Croupier]: Cry me a river :(\n";
			send(fd, answerMessage.c_str(), answerMessage.length(), 0);
			return ;
		}
		else {
			answerMessage = "[Croupier]: You are maybe 1 spin away from the big win.\n";
			send(fd, answerMessage.c_str(), answerMessage.length(), 0);
			return ;
		}
	}

}