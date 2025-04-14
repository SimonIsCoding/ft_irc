#include "../../include/Server.hpp"
#include "../../include/Client.hpp"
#include <sstream>
#include <fstream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <ifaddrs.h>


std::string getContainerIP() {
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char host[NI_MAXHOST];

	if (getifaddrs(&ifaddr) == -1) {
		std::cerr << "getifaddrs failed: " << strerror(errno) << std::endl;
		return "127.0.0.1";
	}
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;

		if (family == AF_INET) {
			s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
						   host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (s != 0) {
				continue;
			}

			std::string ip_addr = host;
			if (ip_addr != "127.0.0.1" && ip_addr.substr(0, 3) != "172") {
				freeifaddrs(ifaddr);
				return ip_addr;
			}
		}
	}

	freeifaddrs(ifaddr);
	return "127.0.0.1";
}

void Server::dcc(int fd, std::istringstream &message)
{
	std::string subcommand;
	message >> subcommand;

	if (subcommand.empty())
	{
		clientLog(fd, "Bad use of command DCC. Use: DCC SEND <nickname> <filename> or DCC ACCEPT <filename>\n");
		return;
	}

	if (subcommand == "SEND")
		dccSend(fd, message);
	else if (subcommand == "ACCEPT" || subcommand == "RESUME")
		dccAccept(fd, message);
	else if (subcommand == "CHAT")
		clientLog(fd, "DCC CHAT not implemented.\n");
	else
		clientLog(fd, "Unknown DCC subcommand. Use: SEND or ACCEPT\n");
}

void Server::dccSend(int fd, std::istringstream &message)
{
	std::string nickname, filename;
	message >> nickname >> filename;

	if (nickname.empty() || filename.empty())
	{
		clientLog(fd, "Bad use of command DCC SEND. Use: DCC SEND <nickname> <filename>\n");
		return;
	}

	int dest_fd = getFdByNickname(nickname);
	if (dest_fd <= 0)
	{
		clientLog(fd, "User not found.\n");
		return;
	}

	std::ifstream file(filename.c_str(), std::ios::binary);
	if (!file)
	{
		std::string error_msg = "File not found or cannot be accessed: " + filename + "\n";
		clientLog(fd, error_msg);
		return;
	}

	file.seekg(0, std::ios::end);
	std::streamsize file_size = file.tellg();
	file.seekg(0, std::ios::beg);

	const std::streamsize MAX_FILE_SIZE = 1 * 1024 * 1024;
	if (file_size > MAX_FILE_SIZE) {
		std::string error_msg = "File too large. Maximum size allowed is 10MB.\n";
		clientLog(fd, error_msg);
		file.close();
		return;
	}

	char* file_buffer = new char[file_size];
	if (!file.read(file_buffer, file_size))
	{
		delete[] file_buffer;
		clientLog(fd, "Error reading file content.\n");
		file.close();
		return;
	}
	file.close();

	std::string simple_filename = filename;
	size_t last_slash = filename.find_last_of("/\\");
	if (last_slash != std::string::npos) {
		simple_filename = filename.substr(last_slash + 1);
	}

	if (fd == dest_fd) {
		std::string out_path = "/tmp/" + simple_filename;
		std::ofstream outfile(out_path.c_str(), std::ios::binary);
		if (outfile) {
			outfile.write(file_buffer, file_size);
			outfile.close();

			clientLog(fd, "File transferred directly to /tmp/" + simple_filename + "\n");
			delete[] file_buffer;
			return;
		}
	}

	int dcc_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (dcc_socket < 0)
	{
		delete[] file_buffer;
		clientLog(fd, "Failed to create DCC socket.\n");
		return;
	}

	int opt = 1;
	if (setsockopt(dcc_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		delete[] file_buffer;
		close(dcc_socket);
		clientLog(fd, "Failed to set socket options.\n");
		return;
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(6668);

	if (bind(dcc_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		delete[] file_buffer;
		close(dcc_socket);
		clientLog(fd, "Failed to bind DCC socket.\n");
		return;
	}

	if (listen(dcc_socket, 1) < 0)
	{
		delete[] file_buffer;
		close(dcc_socket);
		clientLog(fd, "Failed to listen on DCC socket.\n");
		return;
	}

	std::string ip_str = getContainerIP();
	std::cout << "Using IP address: " << ip_str << " for DCC" << std::endl;

	uint32_t ip_addr = ntohl(inet_addr(ip_str.c_str()));
	uint16_t port = 6668;

	std::ostringstream dcc_message;
	dcc_message << "\001DCC SEND " << simple_filename << " "
				<< ip_addr << " " << port << " " << file_size << "\001";

	_dcc_transfers[simple_filename] = DCCTransferInfo(dcc_socket, fd, port);

	if (_dcc_file_contents.find(simple_filename) == _dcc_file_contents.end()) {
		_dcc_file_contents[simple_filename] = std::make_pair(file_buffer, file_size);
	} else {
		delete[] _dcc_file_contents[simple_filename].first;
		_dcc_file_contents[simple_filename] = std::make_pair(file_buffer, file_size);
	}

	std::string privmsg = "PRIVMSG " + nickname + " :" + dcc_message.str() + "\r\n";
	send(dest_fd, privmsg.c_str(), privmsg.length(), 0);

	clientLog(fd, "DCC SEND offer sent to " + nickname + " for file '" + simple_filename + "'. Waiting for acceptance.\n");

	std::cout << "DCC SEND: waiting for connection on port " << port << std::endl;
}

void Server::dccAccept(int fd, std::istringstream &message)
{
	std::string filename, port_str, position_str;
	message >> filename >> port_str >> position_str;

	if (filename.empty())
	{
		clientLog(fd, "Bad use of command DCC ACCEPT. Use: DCC ACCEPT <filename>\n");
		return;
	}

	if (_dcc_transfers.find(filename) == _dcc_transfers.end() ||
		_dcc_file_contents.find(filename) == _dcc_file_contents.end())
	{
		clientLog(fd, "No pending DCC transfer for file '" + filename + "'.\n");
		return;
	}

	int dcc_socket = _dcc_transfers[filename].socket_fd;
	int sender_fd = _dcc_transfers[filename].sender_fd;
	char* file_buffer = _dcc_file_contents[filename].first;
	std::streamsize file_size = _dcc_file_contents[filename].second;

	std::string downloaded_filename = filename + "_downloaded";

	std::ofstream outfile(downloaded_filename.c_str(), std::ios::binary);
	if (!outfile)
	{
		clientLog(fd, "Failed to create output file.\n");
		close(dcc_socket);
		_dcc_transfers.erase(filename);
		delete[] file_buffer;
		_dcc_file_contents.erase(filename);
		return;
	}

	outfile.write(file_buffer, file_size);
	outfile.close();

	clientLog(fd, "File '" + downloaded_filename + "' received successfully.\n");
	clientLog(sender_fd, "File '" + filename + "' sent successfully.\n");

	close(dcc_socket);
	_dcc_transfers.erase(filename);
	delete[] file_buffer;
	_dcc_file_contents.erase(filename);

}