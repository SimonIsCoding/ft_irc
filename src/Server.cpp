#include "../include/Server.hpp"
#include "../include/Client.hpp"

IRCServer::IRCServer(int _port, std::string pass) : _port(_port), _password(pass) {
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
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, this->_server_fd, &ev) == -1) {
		throw std::runtime_error("Failed to add server socket to epoll");
	}
}

IRCServer::~IRCServer() {
	// Clean up all _clients
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		delete (*it).second;
	}
	_clients.clear();
	close(_server_fd);
	close(_epoll_fd);
}

void IRCServer::run() {
	std::cout << "IRC Server running on port " << _port << std::endl;
	struct epoll_event events[MAX_EVENTS];

	while (true) {
		int	nb_events = epoll_wait(this->_epoll_fd, events, MAX_EVENTS, -1);
		std::cout << "hola\n";
		if (nb_events == -1)
		{
			std::cerr << "epoll_wait failed\n";
			continue ;
		}
		for (int i = 0; i < nb_events; i++)
		{
			if (events[i].data.fd == STDIN_FILENO)
				throw std::runtime_error("Exited program");
			else if (events[i].data.fd == this->_server_fd)//receive new connection
				handleNewConnection();
			else
			{
				handleClientMessage(_clients[events[i].data.fd]);
				break;
				// for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
				// 	if ((*it)->getSocket() == events[i].data.fd) {
				// 		handleClientMessage(*it);
				// 		break;
				// 	}
				// }
			}
		}


	}
}

void IRCServer::handleNewConnection() {
	int client_fd;
	int addrlen = sizeof(address);

	if ((client_fd = accept(_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)//create client fd
	{
		std::cerr << "Accept failed" << std::endl;
		return;
	}

	// Add new client
	Client* new_client = new Client(client_fd);
	std::cout << "new allocation made\n";
	_clients.insert(std::make_pair(client_fd, new_client));

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = client_fd;
	if (epoll_ctl(this->_epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
		std::cerr << "fail to add client socket to epoll\n";
		return ;
	}

	std::cout << "New client connected. Total _clients: " << _clients.size() << std::endl;
}

void IRCServer::handleClientMessage(Client* client) {
	std::string message = client->receiveMessage();
	if (message.empty()) {
		std::cout << "Client disconnected" << std::endl;
		removeClient(client);
		return;
	}

	std::cout << "Received from client " << client->getNickname() << ": " << message << std::endl;

	// Broadcast message to all _clients
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if ((*it).second != client) {
			(*it).second->sendMessage(message);
		}
	}
}

void IRCServer::removeClient(Client* client) {
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, client->getSocket(), NULL);

	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		if (it->second == client) {
			_clients.erase(it);
			delete (*it).second;
			break;
		}
	}
}

