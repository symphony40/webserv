#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "Utils.hpp"
#include "BlocServer.hpp"

#define BACKLOGS 100

class Socket {
private:
	int _fd;
	std::string _ipAddr;
	unsigned int _port;
	std::vector<BlocServer> *_servers;
	struct sockaddr_in _addr;

public:
	Socket();
	Socket(int fd, std::string ip, unsigned int port, std::vector<BlocServer> *servers);
	Socket(Socket const &obj);
	~Socket();
	Socket &operator=(const Socket &obj);

	std::string getIp() const;
	unsigned int getPort() const;
	int getFd() const;
	std::vector<BlocServer>* getServers() const;
	struct sockaddr_in getAddr() const;
};

#endif
