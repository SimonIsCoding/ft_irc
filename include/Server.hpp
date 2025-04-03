#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <iostream>

class IRCServer {
private:
    int server_fd;
    int port;
    struct sockaddr_in address;
    static const int BUFFER_SIZE = 1024;

    void handleClient(int client_socket);

public:
    IRCServer(int port);
    void run();
};

#endif 