#include "../include/Server.hpp"
#include "../include/Client.hpp"

Server::Server(int _port, std::string pass) : _port(_port), _password(pass) {
	// Create socket
	if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		throw std::runtime_error("Socket creation failed");
	}

	// Configure address
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(_port);

	// Set socket options
	int opt = 1;
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
		throw std::runtime_error("Setsockopt failed");
	}

	// Bind socket
	if (bind(_server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		throw std::runtime_error("Bind failed");
	}

	// Listen for connections
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
	// Clean up all _clients
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		delete (*it).second;
	}
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
		delete (*it).second;
	}
	_clients.clear();
	_channels.clear();
	close(_server_fd);
	close(_epoll_fd);
}

void signal_handler(int signal) {
	(void) signal;
	throw std::logic_error("Server shutdown");
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
	while (true) {
		int	nb_events = epoll_wait(this->_epoll_fd, events, MAX_EVENTS, -1);
		if (nb_events == -1)
		{
			std::cerr << "epoll_wait failed\n";
			continue ;
		}
		for (int i = 0; i < nb_events; i++)
		{
			if (events[i].data.fd == STDIN_FILENO){
				ServerExit();
				break;
			}
			else if (events[i].data.fd == this->_server_fd)//receive new connection
				handleNewConnection();
			else
			{
				handleClientMessage(events[i].data.fd);
				break;
			}
		}
	}
}

void Server::ServerExit(){
	std::string stdin_content;

	getline(std::cin, stdin_content);
	if (stdin_content == "exit")
		throw std::logic_error("Server shutdown");
}

void Server::handleNewConnection() {
	int client_fd;
	int addrlen = sizeof(address);

	if ((client_fd = accept(_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)//create client fd
	{
		std::cerr << "Accept failed" << std::endl;
		return;
	}

	// Add new client
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
	if (message.empty()) {
		std::cout << "Client disconnected" << std::endl;
		removeClient(client_fd);
		return;
	}
	std::istringstream strm_msg(message);
	parsing(client_fd, strm_msg);
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
	message = "[SERVER]: " + message;
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
	if (content[0] != ':')
		return (false);
	for (size_t i = 1; i < content.size(); ++i)
		if (!std::isalpha(content[i]) && content[i] != ' ')
			return (false);
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