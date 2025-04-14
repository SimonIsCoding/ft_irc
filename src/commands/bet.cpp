#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include "../../include/Channel.hpp"
#include <limits>

#define RED "\e[1;31m"
#define GRN "\e[1;32m"
#define NC	"\e[0m"

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

	if (moneyAmount < 0)
	{
		answerMessage =  "[Croupier]: You can't bet negative values IDIOT 🤣🫵\n";
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
	bool isHead = random > 50 ? true : false;
	spinCoin(fd, isHead, side);
	int clientMoney = _clients[fd]->getMoney();
	_clients[fd]->setMoney(clientMoney - moneyAmount);
	if (side == "tail" && !isHead) {
		_clients[fd]->setMoney(clientMoney + moneyAmount);
		std::stringstream ss;
		ss << "You now have " << _clients[fd]->getMoney() << "$\n";
		clientLog(fd, ss.str());
		answerMessage = "[Croupier]: You won for this round, try again and all in for the lore.\n";
		send(fd, answerMessage.c_str(), answerMessage.length(), 0);
		return ;
	}
	else if (side == "head" && isHead) {
		_clients[fd]->setMoney(clientMoney + moneyAmount);
		std::stringstream ss;
		ss << "You now have " << _clients[fd]->getMoney() << "$\n";
		clientLog(fd, ss.str());
		answerMessage = "[Croupier]: You won for this round, try again and all in for the lore.\n";
		send(fd, answerMessage.c_str(), answerMessage.length(), 0);
		return ;
	}
	else if (side == "both") {
		int max_int = std::numeric_limits<int>::max();
		int BOTHAMOUNT = clientMoney * 2 < max_int ? clientMoney * 2 : 1000000000;
		_clients[fd]->setMoney(BOTHAMOUNT);
		std::stringstream ss;
		ss << "You now have " << _clients[fd]->getMoney() << "$\n";
		clientLog(fd, ss.str());
		answerMessage = "[Croupier]: You won for this round, try again and all in for the lore.\n";
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

void Server::spinCoin(int fd, bool isHead, std::string &guess)
{
	std::ifstream head_file("head.txt");
	std::ifstream tail_file("tail.txt");
	int random = (rand() % 10) + 5;
    if (!head_file || !tail_file) {
        return ;
    }
	std::stringstream buffer;

	buffer << head_file.rdbuf();
	std::string head = buffer.str();
	buffer.str("");
	buffer.clear();

	buffer << tail_file.rdbuf();
	std::string tail = buffer.str();

	int wait_time = 100000;
	for (int i = 0 ; i < random ; i++)
	{
		clientLog(fd, head);
		usleep(wait_time);
		clientLog(fd, tail);
		usleep(wait_time);
		wait_time += 40000;
	}
	if ((guess == "head" && isHead) || (guess == "tail" && !isHead))
		send(fd, GRN, std::string(GRN).length(), 0);
	else
		send(fd, RED, std::string(RED).length(), 0);
	if (isHead)
		clientLog(fd, head);
	else
		clientLog(fd, tail);
	send(fd, NC, std::string(NC).length(), 0);
	return;
}
