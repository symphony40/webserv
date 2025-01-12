#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

#include "BlockConfigServer.hpp"
#include "Utils.hpp"

#define BACKLOGS 100

class Socket {
private:
	int _fd;
	std::string _ipAddr;
	unsigned int _port;
	std::vector<BlockConfigServer> *_servers;
	struct sockaddr_in _addr;

public:
	Socket();
	Socket(int fd, std::string ip, unsigned int port, std::vector<BlockConfigServer> *servers);
	Socket(Socket const &obj);
	~Socket();
	Socket &operator=(Socket const &obj);

	std::string getIp() const;
	unsigned int getPort() const;
	int getFd() const;
	std::vector<BlockConfigServer>* getServers() const;
	struct sockaddr_in getAddr() const;
};

#endif
