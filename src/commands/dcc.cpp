#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include <sstream>
#include <fstream>
#include <sys/stat.h>

void Server::dcc(int fd, std::istringstream &strm_msg) {
	std::string type;
	std::string target;
	std::string filename;
	std::string ip;
	int port;
	std::streamsize file_size;

	strm_msg >> type >> target >> filename >> ip >> port >> file_size;

	if (type.empty() || target.empty() || filename.empty() || ip.empty() || port <= 0 || file_size <= 0) {
		clientLog(fd, "Bad DCC command syntax. Usage: DCC SEND|RECV <nickname> <filename> <ip> <port> <size>\n");
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

		if (_clients[fd]->startDCCSend(filename, ip, port, st.st_size)) {
			clientLog(fd, "DCC SEND started successfully.\n");
		} else {
			clientLog(fd, "Failed to start DCC SEND.\n");
		}
	} else if (type == "RECV") {
		if (_clients[fd]->startDCCReceive(filename, ip, port, file_size)) {
			clientLog(fd, "DCC RECV started successfully.\n");
		} else {
			clientLog(fd, "Failed to start DCC RECV.\n");
		}
	} else {
		clientLog(fd, "Invalid DCC type. Use SEND or RECV.\n");
	}
} 