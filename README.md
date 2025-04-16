# IRC Server Project

This project implements an IRC (Internet Relay Chat) server using C++98 and Linux system calls. The server handles multiple client connections, channels, and various IRC commands.

## Socket Programming Basics

### What is a Socket?
A socket is an endpoint for communication between two machines over a network. It's a software construct that provides a bidirectional communication channel between processes, either on the same machine or across different machines. In this project, we use TCP sockets which provide:
- Reliable, ordered, and error-checked delivery of data
- Connection-oriented communication
- Stream-based data transfer

### What is epoll?
epoll is a Linux-specific I/O event notification mechanism that efficiently monitors multiple file descriptors for events. It's an improvement over older mechanisms like select() and poll() because:
- It scales better with large numbers of file descriptors
- It uses a more efficient event notification system
- It doesn't require scanning all file descriptors on each call
- It supports edge-triggered and level-triggered modes

The epoll API consists of three main system calls:
1. `epoll_create()`: Creates an epoll instance
2. `epoll_ctl()`: Registers/unregisters file descriptors
3. `epoll_wait()`: Waits for events on registered file descriptors

### Key Socket Functions

1. **socket()**
   - Creates a new socket
   - Usage: `socket(AF_INET, SOCK_STREAM, 0)`
   - Returns a file descriptor for the new socket
   - AF_INET: IPv4 protocol family
   - SOCK_STREAM: TCP socket type

2. **setsockopt()**
   - Sets socket options
   - Usage: `setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))`
   - SO_REUSEADDR: Allows reuse of local addresses
   - Important for server restarts to avoid "Address already in use" errors

3. **bind()**
   - Binds a socket to an address and port
   - Usage: `bind(socket_fd, (struct sockaddr *)&address, sizeof(address))`
   - Associates the socket with a specific port and IP address
   - Returns 0 on success, -1 on error

4. **listen()**
   - Marks the socket as passive (server socket)
   - Usage: `listen(socket_fd, backlog)`
   - backlog: Maximum length of the queue of pending connections
   - Returns 0 on success, -1 on error

5. **epoll_create()**
   - Creates an epoll instance
   - Usage: `epoll_create1(0)`
   - Returns a file descriptor for the epoll instance
   - Used for efficient I/O event monitoring

6. **fcntl()**
   - Manipulates file descriptor properties
   - Usage: `fcntl(fd, F_SETFL, O_NONBLOCK)`
   - Sets socket to non-blocking mode
   - Important for asynchronous I/O operations

7. **accept()**
   - Accepts a new connection
   - Usage: `accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)`
   - Returns a new socket descriptor for the accepted connection
   - Creates a new socket for client communication

8. **epoll_ctl()**
   - Controls an epoll instance
   - Usage: `epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev)`
   - Adds/modifies/removes file descriptors to/from the epoll instance
   - EPOLL_CTL_ADD: Adds a new file descriptor

9. **epoll_wait()**
   - Waits for I/O events
   - Usage: `epoll_wait(epoll_fd, events, MAX_EVENTS, timeout)`
   - Returns the number of ready file descriptors
   - Used for event-driven programming

## Building and Running

1. Clone the repository
2. Run `make` to build the project
3. Start the server: `./ircserv <port> <password>`

Example:
```bash
./ircserv 6667 password123
```

## Connecting to the Server

### Using Netcat
1. Connect to the server:
```bash
nc localhost <port>
```

2. Register with the server:
```
PASS <password>
NICK <nickname>
USER <username> <hostname> <servername> :<realname>
```

Example:
```bash
nc localhost 6667
PASS password123
NICK user1
USER user1 0 * :John Doe
```

### Using the Official IRC Client (sic)
1. Install sic:
```bash
sudo apt-get install sic
```

2. Connect to the server:
```bash
sic -h localhost -p <port> -n <nickname> -k <password>
```

Example:
```bash
sic -h localhost -p 6667 -n user1 -k password123
```

<!-- 3. Using commands with sic:
:<command> <argument>

Example:
:JOIN #channel_name
:PRIVMSG <nickname> <content> -->

## IRC Commands

### Basic Commands

#### PASS (Set password to enter in the server - used only once at the beginning)
- **Purpose**: Set your password
- **sic Syntax**: `:PASS <password>`
- **netcat Syntax**: `PASS <password>`
- **Example**:
  ```bash
  # sic
  :PASS john_doe
  
  # netcat
  PASS john_doe
  ```

#### NICK (Set Nickname - used only once at the beginning)
- **Purpose**: Set your nickname
- **sic Syntax**: `:NICK <new_nickname>`
- **netcat Syntax**: `NICK <new_nickname>`
- **Example**:
  ```bash
  # sic
  :NICK john_doe
  
  # netcat
  NICK john_doe
  ```

  #### USER (Set Username - used only once at the beginning)
- **Purpose**: Set your Username
- **sic Syntax**: `:USER <username> <hostname> <servername> :<realname>`
- **netcat Syntax**: `USER <username> <hostname> <servername> :<realname>`
- **Example**:
  ```bash
  # sic
  :USER user1 0 * :John Doe
  
  # netcat
  USER user1 0 * :John Doe
  ```

#### JOIN (Join Channel)
- **Purpose**: Join a channel
- **sic Syntax**: `:JOIN \#<channel_name>`
- **netcat Syntax**: `JOIN \#<channel_name>`
- **Example**:
  ```bash
  # sic
  :JOIN #general
  
  # netcat
  JOIN #general
  ```

<!-- #### PART (Leave Channel)
- **Purpose**: Leave a channel
- **sic Syntax**: `:PART #<channel_name>`
- **netcat Syntax**: `PART #<channel_name>`
- **Example**:
  ```bash
  # sic
  :PART #general
  
  # netcat
  PART #general
  ``` -->

#### PRIVMSG (Private Message)
- **Purpose**: Send a private message to a user or channel
- **sic Syntax**: `:PRIVMSG <target> <message>`
- **netcat Syntax**: `PRIVMSG <target> <message>`
- **Example**:
  ```bash
  # sic
  :PRIVMSG #general Hello everyone!
  :PRIVMSG john_doe Hi there!
  
  # netcat
  PRIVMSG #general Hello everyone!
  PRIVMSG john_doe Hi there!
  ```

<!-- #### QUIT (Disconnect)
- **Purpose**: Disconnect from the server
- **sic Syntax**: `:QUIT :<message>`
- **netcat Syntax**: `QUIT :<message>`
- **Example**:
  ```bash
  # sic
  :QUIT :Goodbye everyone!
  
  # netcat
  QUIT :Goodbye everyone!
  ``` -->

### Channel Management Commands

#### TOPIC (Set Channel Topic)
- **Purpose**: Set or view channel topic
- **sic Syntax**: `:TOPIC #<channel_name> :<new_topic>`
- **netcat Syntax**: `TOPIC #<channel_name> :<new_topic>`
- **Example**:
  ```bash
  # sic
  :TOPIC #general :Welcome to the general chat!
  
  # netcat
  TOPIC #general :Welcome to the general chat!
  ```

#### KICK (Remove User from Channel)
- **Purpose**: Remove a user from a channel
- **sic Syntax**: `:KICK #<channel_name> <nickname> :<reason>`
- **netcat Syntax**: `KICK #<channel_name> <nickname> :<reason>`
- **Example**:
  ```bash
  # sic
  :KICK #general john_doe :Being disruptive
  
  # netcat
  KICK #general john_doe :Being disruptive
  ```

#### MODE (Set Channel Modes)
- **Purpose**: Set channel modes (operator, voice, etc.)
- **sic Syntax**: `:MODE #<channel_name> <mode> <nickname>`
- **netcat Syntax**: `MODE #<channel_name> <mode> <nickname>`
- **Example**:
  ```bash
  # sic
  :MODE #general +i john_doe  # Make john_doe an operator
  :MODE #general +t john_doe  # Make john_doe an operator
  :MODE #general +k john_doe  # Make john_doe an operator
  :MODE #general +o john_doe  # Make john_doe an operator
  :MODE #general -o john_doe  # Remove john_doe as operator
  :MODE #general +l john_doe  # Make john_doe an operator
> [!NOTE] MODE can take several letter at the same time, as long as the argument written after are well configured.
  :MODE #general +l john_doe  # Make john_doe an operator
  
  # netcat
  MODE #general +o john_doe # Make john_doe an operator
  ```

### Bonus Commands

#### Casino Commands
- **Purpose**: Play casino games
- **sic Syntax**: `:CASINO <command> <arguments>`
- **netcat Syntax**: `CASINO <command> <arguments>`
- **Examples**:
  ```bash
  # sic
  :CASINO JOIN
  :CASINO BET 100
  :CASINO GAMES
  
  # netcat
  CASINO JOIN
  CASINO BET 100
  CASINO GAMES
  ```

#### DCC (File Transfer) Commands
- **Purpose**: Transfer files between users
- **sic Syntax**: `:DCC <command> <arguments>`
- **netcat Syntax**: `DCC <command> <arguments>`
- **Examples**:
  ```bash
  # sic
  :DCC SEND john_doe file.txt
  :DCC ACCEPT john_doe
  :DCC CANCEL john_doe
  
  # netcat
  DCC SEND john_doe file.txt
  DCC ACCEPT john_doe
  DCC CANCEL john_doe
  ```

## Main Classes and Methods

### Server Class
- **Constructor**: `Server(int port, std::string password)`
  - Initializes the server socket
  - Sets up epoll for event handling
  - Configures socket options

- **Key Methods**:
  - `run()`: Main server loop
  - `handleClientMessage(int client_fd)`: Processes client messages
  - `removeClient(int fd)`: Removes a client from the server
  - `clientLog(int fd, std::string message)`: Logs client activities

### Client Class
- **Constructor**: `Client(int fd)`
  - Initializes a new client connection
  - Sets up client-specific properties

- **Key Methods**:
  - `getFd()`: Returns client's file descriptor
  - `getNickname()`: Returns client's nickname
  - `setNickname(std::string nickname)`: Sets client's nickname
  - `sendMessage(std::string message)`: Sends a message to the client

### Channel Class
- **Constructor**: `Channel(std::string name)`
  - Creates a new channel
  - Initializes channel properties

- **Key Methods**:
  - `addClient(Client* client)`: Adds a client to the channel
  - `removeClient(Client* client)`: Removes a client from the channel
  - `broadcastMessage(std::string message)`: Sends a message to all channel members
  - `getClients()`: Returns list of channel clients


## Dependencies
- C++ compiler with C++98 support
- Linux system (for epoll)
- Make
- sic (optional, for official client testing)
- netcat (optional, for basic testing)

## Security Considerations
- Password protection for server access
- Input validation for all commands
- Proper client disconnection handling
- Memory leak prevention in destructors