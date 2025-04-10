#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <arpa/inet.h>

void Server::dcc(int fd, std::istringstream &strm_msg) {
	std::string type;
	std::string target;
	std::string filename;
	std::string ip;
	int port;
	std::streamsize file_size;

	strm_msg >> type >> target >> filename >> ip >> port >> file_size;

	if (type.empty() || target.empty() || filename.empty() || ip.empty() || port <= 0 || file_size <= 0) {
		clientLog(fd, "Bad DCC command syntax. Usage: DCC SEND <nickname> <filename>\n");
		return;
	}

	int target_fd = getFdByNickname(target);
	if (target_fd == -1) {
		clientLog(fd, "Target user not found.\n");
		return;
	}

	if (type == "SEND") {
		// Check if file exists and get its size
		struct stat st;
		if (stat(filename.c_str(), &st) != 0) {
			clientLog(fd, "File not found.\n");
			return;
		}

		// Get local IP address
		struct sockaddr_in addr;
		socklen_t addr_len = sizeof(addr);
		getsockname(_clients[fd]->getSocket(), (struct sockaddr*)&addr, &addr_len);
		std::string local_ip = inet_ntoa(addr.sin_addr);
		
		// Convert IP to DCC format (dotted decimal to integer)
		unsigned long ip_long = inet_addr(local_ip.c_str());
		ip_long = ntohl(ip_long); // Convert to network byte order

		// Send DCC SEND offer to target
		std::string dcc_offer = "PRIVMSG " + target + " :\001DCC SEND " + filename + " " + 
			std::to_string(ip_long) + " " + std::to_string(port) + " " + 
			std::to_string(st.st_size) + "\001\r\n";
		send(target_fd, dcc_offer.c_str(), dcc_offer.length(), 0);

		// Start listening for DCC connection
		if (_clients[fd]->startDCCSend(filename, local_ip, port, st.st_size)) {
			clientLog(fd, "DCC SEND offer sent. Waiting for acceptance...\n");
		} else {
			clientLog(fd, "Failed to start DCC SEND.\n");
		}
	} else {
		clientLog(fd, "Invalid DCC type. Use SEND.\n");
	}
} 