#include "Socket.hpp"

Socket::Socket() : _fd(-1) {}

Socket::Socket(int fd, std::string ipAddr, unsigned int port, std::vector<BlockConfigServer> *servers) : _fd(fd), _ipAddr(ipAddr), _port(port), _servers(servers) {
	Logger::log(Logger::INFO, "Initializing socket on %s:%d", ipAddr.c_str(), port);
	try {
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(port);
		_addr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
		int optval = 1;
		Utils::tryCall(setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)), "[Socket] Failed to set socket options");
		Utils::tryCall(bind(_fd, (struct sockaddr *)&_addr, sizeof(_addr)), "[Socket] Failed to bind socket");
		Utils::tryCall(listen(_fd, BACKLOGS), "[Socket] Failed to listen on socket");
	} catch (std::exception &e) {
		if (_fd != -1) {
			Utils::tryCall(close(_fd), "[Socket] Failed to close socket", false);
		}
		Logger::log(Logger::FATAL, "Failed to initialize socket on %s:%d", ipAddr.c_str(), port);
	}
}

Socket::Socket(Socket const &obj) {	*this = obj; }

Socket::~Socket() {
	if (_fd != -1) {
		Utils::tryCall(close(_fd), "Failed to close socket", false);
	}
}

Socket &Socket::operator=(Socket const &obj) {
	if (this != &obj) {
		_ipAddr = obj._ipAddr;
		_port = obj._port;
		_fd = obj._fd;
		_servers = obj._servers;
		_addr = obj._addr;
	}
	return *this;
}

int Socket::getFd() const { return _fd; }

std::string Socket::getIp() const { return _ipAddr; }

std::vector<BlockConfigServer> *Socket::getServers() const { return _servers; }

struct sockaddr_in Socket::getAddr() const { return _addr; }

unsigned int Socket::getPort() const { return _port; }
