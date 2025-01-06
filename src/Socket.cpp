#include "Socket.hpp"

Socket::Socket() : _fd(-1) {}

Socket::Socket(int fd, std::string ipAddr, unsigned int port, std::vector<BlocServer> *servers) : _fd(fd), _ipAddr(ipAddr), _port(port), _servers(servers) {
	Logger::log(Logger::INFO, "Initializing socket on %s:%d", ipAddr.c_str(), port);
	try {
		this->_addr.sin_family = AF_INET;
		this->_addr.sin_port = htons(port);
		this->_addr.sin_addr.s_addr = inet_addr(ipAddr.c_str());
		Utils::protectedCall(fcntl(this->_fd, F_SETFL, O_NONBLOCK), "[Socket] Failed to set socket to non-blocking");
		int optval = 1;
		Utils::protectedCall(setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)), "[Socket] Failed to set socket options");
		Utils::protectedCall(bind(this->_fd, (struct sockaddr *)&this->_addr, sizeof(this->_addr)), "[Socket] Failed to bind socket");
		Utils::protectedCall(listen(this->_fd, BACKLOGS), "[Socket] Failed to listen on socket");
	} catch (std::exception &e) {
		if (this->_fd != -1) {
			Utils::protectedCall(close(this->_fd), "[Socket] Failed to close socket", false);
		}
		Logger::log(Logger::FATAL, "Failed to initialize socket on %s:%d", ipAddr.c_str(), port);
	}
}

Socket::Socket(const Socket &obj) {	*this = obj; }

Socket::~Socket() {
	if (this->_fd != -1) {
		Utils::protectedCall(close(this->_fd), "Failed to close socket", false);
	}
}

Socket &Socket::operator=(const Socket &obj) {
	if (this != &obj) {
		this->_ipAddr = obj._ipAddr;
		this->_port = obj._port;
		this->_fd = obj._fd;
		this->_servers = obj._servers;
		this->_addr = obj._addr;
	}
	return *this;
}

int Socket::getFd() const { return _fd; }

std::string Socket::getIp() const { return _ipAddr; }

std::vector<BlocServer> *Socket::getServers() const { return _servers; }

struct sockaddr_in Socket::getAddr() const { return _addr; }

unsigned int Socket::getPort() const { return _port; }
