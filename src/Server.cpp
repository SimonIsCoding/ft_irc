#include "../include/Server.hpp"
#include "../include/Client.hpp"

IRCServer::IRCServer(int port) : port(port), max_fd(0) {
    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        throw std::runtime_error("Socket creation failed");
    }

    // Configure address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Set socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        throw std::runtime_error("Setsockopt failed");
    }

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Bind failed");
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        throw std::runtime_error("Listen failed");
    }

    // Initialize fd_set
    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);
    max_fd = server_fd;
}

IRCServer::~IRCServer() {
    // Clean up all clients
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        delete *it;
    }
    clients.clear();
    close(server_fd);
}

void IRCServer::run() {
    std::cout << "IRC Server running on port " << port << std::endl;
    
    while (true) {
        fd_set read_fds = master_fds;
        
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            std::cerr << "Select failed" << std::endl;
            continue;
        }

        // Check for new connections
        if (FD_ISSET(server_fd, &read_fds)) {
            handleNewConnection();
        }

        // Check for client messages
        for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end();) {
            Client* client = *it;
            if (FD_ISSET(client->getSocket(), &read_fds)) {
                handleClientMessage(client);
            }
            ++it;
        }
    }
}

void IRCServer::handleNewConnection() {
    int new_socket;
    int addrlen = sizeof(address);
    
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        std::cerr << "Accept failed" << std::endl;
        return;
    }

    // Add new client
    Client* new_client = new Client(new_socket);
    clients.push_back(new_client);
    
    // Update fd_set
    FD_SET(new_socket, &master_fds);
    if (new_socket > max_fd) {
        max_fd = new_socket;
    }

    std::cout << "New client connected. Total clients: " << clients.size() << std::endl;
}

void IRCServer::handleClientMessage(Client* client) {
    std::string message = client->receiveMessage();
    if (message.empty()) {
        std::cout << "Client disconnected" << std::endl;
        removeClient(client);
        return;
    }

    std::cout << "Received from client " << client->getNickname() << ": " << message << std::endl;
    
    // Broadcast message to all clients
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (*it != client) {
            (*it)->sendMessage(message);
        }
    }
}

void IRCServer::removeClient(Client* client) {
    // Remove from fd_set
    FD_CLR(client->getSocket(), &master_fds);
    
    // Remove from clients vector
    for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (*it == client) {
            delete *it;
            clients.erase(it);
            break;
        }
    }

    // Update max_fd if needed
    if (client->getSocket() == max_fd) {
        max_fd = server_fd;
        for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
            if ((*it)->getSocket() > max_fd) {
                max_fd = (*it)->getSocket();
            }
        }
    }
}
