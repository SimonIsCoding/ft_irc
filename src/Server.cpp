#include "../include/Server.hpp"

IRCServer::IRCServer(int port) : port(port) {
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
}

void IRCServer::run() {
    std::cout << "IRC Server running on port " << port << std::endl;
    
    while (true) {
        int new_socket;
        int addrlen = sizeof(address);
        
        // Accept new connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        std::cout << "New client connected" << std::endl;
        handleClient(new_socket);
    }
}

void IRCServer::handleClient(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::fill_n(buffer, BUFFER_SIZE, 0);
    
    while (true) {
        // Read client request
        int valread = read(client_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            std::cout << "Client disconnected" << std::endl;
            break;
        }

        // Process the request
        std::string request(buffer, valread);
        std::cout << "Received: " << request << std::endl;

        // Send response (echo for now)
        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
    }

    close(client_socket);
}
