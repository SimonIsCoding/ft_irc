#include "../include/Server.hpp"
#include "../include/Client.hpp"
#include <cerrno>

Server::Server(int _port, std::string pass) : _port(_port), _password(pass) {
	if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		throw std::runtime_error("Socket creation failed");
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(_port);

	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		throw std::runtime_error("Setsockopt failed");
	}

	if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		throw std::runtime_error("Bind failed");
	}

	if (listen(_server_fd, 3) < 0) {
		throw std::runtime_error("Listen failed");
	}

	if ((this->_epoll_fd = epoll_create1(0)) == -1) {
		throw std::runtime_error("epoll create error");
	}

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = _server_fd;
	fcntl(this->_server_fd, F_SETFL, O_NONBLOCK);
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, this->_server_fd, &ev) == -1) {
		throw std::runtime_error("Failed to add server socket to epoll");
	}
}

Server::~Server() {
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		delete (*it).second;
	}

	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		delete (*it).second;
	}

	for (std::map<std::string, std::pair<char*, std::streamsize> >::iterator it = _dcc_file_contents.begin();
	     it != _dcc_file_contents.end(); ++it) {
		delete[] it->second.first;
	}

	for (std::map<std::string, DCCTransferInfo>::iterator it = _dcc_transfers.begin();
	     it != _dcc_transfers.end(); ++it) {
		close(it->second.socket_fd);
	}

	_clients.clear();
	_channels.clear();
	_dcc_transfers.clear();
	_dcc_file_contents.clear();
	close(_server_fd);
	close(_epoll_fd);
}

static volatile sig_atomic_t g_running = 1;

void signal_handler(int signal) {
	(void)signal;
	g_running = 0;
}

void Server::run() {
	std::cout << "IRC Server running on port " << _port << std::endl;
	struct epoll_event events[MAX_EVENTS];

	std::signal(SIGINT, signal_handler);
	std::signal(SIGTERM, signal_handler);
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = STDIN_FILENO;
	fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev) == -1) {
		throw std::runtime_error("Failed to add standard input to epoll");
	}
	createCasino();
	while (g_running) {
		int nb_events = epoll_wait(this->_epoll_fd, events, MAX_EVENTS, -1);
		if (nb_events == -1) {
			if (errno == EINTR)
				continue;
			std::cerr << "epoll_wait failed\n";
			continue;
		}
		for (int i = 0; i < nb_events; i++) {
			if (events[i].data.fd == STDIN_FILENO) {
				ServerCommand();
				break;
			}
			else if (events[i].data.fd == this->_server_fd)
				handleNewConnection();
			else {
				handleClientMessage(events[i].data.fd);
				break;
			}
		}
	}
	std::cout << "Server shutting down gracefully..." << std::endl;
}

void Server::ServerCommand() {
	std::string stdin_content;

	getline(std::cin, stdin_content);
	if (stdin_content.empty())
		return ;
	if (stdin_content == "exit")
		throw std::logic_error("Server shutdown");
	if (stdin_content[0] == ':')
	{
		std::string name = stdin_content.substr(1, stdin_content.find(' ') - 1);
		std::string content = stdin_content.substr(stdin_content.find(' ') + 1);
		if (name[0] == ':')
			name = name.substr(1);
		int client_fd = getFdByNickname(name);
		if (client_fd == -1) {
			std::cout << "User " + name + " not found." << std::endl;
			return ;
		}
		std::istringstream strm_msg(content);
		parsing(client_fd, strm_msg);
		return ;
	}
	std::cout << "Command " << stdin_content << " not known."<< std::endl;
}

void Server::handleNewConnection() {
	int client_fd;
	int addrlen = sizeof(address);

	if ((client_fd = accept(_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
	{
		std::cerr << "Accept failed" << std::endl;
		return;
	}

	Client* new_client = new Client(client_fd);
	_clients.insert(std::make_pair(client_fd, new_client));

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = client_fd;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
		std::cerr << "fail to add client socket to epoll\n";
		return ;
	}
	dealerMessage(client_fd);

	std::cout << "New client connected. Total _clients: " << _clients.size() << std::endl;
}

void Server::handleClientMessage(int client_fd) {
	std::string message = _clients[client_fd]->receiveMessage();

	message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());
	std::size_t pos = message.find("\n");
	if (pos == std::string::npos) {
		if (message.empty()) {
			std::cout << "Client disconnected" << std::endl;
			removeClient(client_fd);
			return;
		}
		return ;
	}
	else {
		while (pos != std::string::npos) {
			std::string complete_message = message.substr(0, pos);
			message = message.substr(pos + 1);
			_clients[client_fd]->setPartial(message);
			std::istringstream strm_msg(complete_message);
			parsing(client_fd, strm_msg);
			pos = message.find("\n");
		}
	}
}

void Server::removeClient(int fd) {
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, _clients[fd]->getSocket(), NULL);

	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->first == fd) {
			delete it->second;
			_clients.erase(fd);
			break;
		}
	}
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->second->isMember(fd))
			it->second->deleteMember(fd);
		if (it->second->isOperator(fd))
			it->second->rmOperator(fd);
	}
}

void Server::clientLog(int fd, std::string message){
	message = "IRCSERVER: " + message;
	send(fd, message.c_str(), message.length(), 0);
}

bool	Server::checkEmpty(std::istringstream &content)
{
	std::string tmp;

	content >> tmp;
	if (tmp.empty())
		return (true);
	return (false);
}

bool	Server::check_realname_syntax(const std::string &content)
{
	bool check = false;
	size_t len = content.size() - 1;

	std::size_t pos = content.find("\r");
	if (pos == std::string::npos) {
		len = content.size();
	}

	if (content[0] != ':')
		return (false);
	for (size_t i = 1; i < len; ++i)
	{
		check = true;
		if (!std::isalpha(content[i]) && content[i] != ' ')
			return (false);
	}
	if (check == false)
		return false;
	return (true);
}

void Server::createChannel(Client *creator, const std::string &name)
{
	Channel* newchannel = new Channel(name);

	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if (it->first == name)
			return (clientLog(creator->getSocket(), "Channel already exists.\n"));
	}
	_channels.insert(std::make_pair(name, newchannel));
	newchannel->addOperator(creator);
	newchannel->addMember(creator);
}

int	Server::getFdByNickname(std::string &nickname){
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname){
			return it->first;
		}
	}
	return -1;
}

void Server::createCasino() {
	Client* new_client = new Client(10001);
	_clients.insert(std::make_pair(10001, new_client));
	new_client->setNickname("Croupier");
	createChannel(new_client, "#casino");
}

void Server::commandLog(std::string command, bool success) {
	if (success)
		std::cout << "\033[1;32mCommand " + command + " has been done successfully.\033[0m\n" << std::endl;
	else
		std::cout << "\033[1;31mFail to execute " + command << ".\033[0m\n" << std::endl;
}