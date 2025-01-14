#ifndef SERVER_HPP
#define SERVER_HPP

#include <algorithm>
#include <arpa/inet.h>
#include <iostream>
#include <map>
#include <sys/epoll.h>
#include <vector>

#include "BlockConfigServer.hpp"
#include "Client.hpp"
#include "ConfigParser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Socket.hpp"
#include "Utils.hpp"

#define SERVER_DEFAULT_EPOLL_WAIT 500
#define TIMEOUT_CHECK_INTERVAL 5
#define INACTIVITY_TIMEOUT 60
#define TIMEOUT_CGI 30 

//#define TIMEOUT_CGI_CHECK_INTERVAL 1 // seconds

enum ServerState {
	SERVER_STATE_INIT,
	SERVER_STATE_READY,
	SERVER_STATE_RUN,
	SERVER_STATE_STOP
};

class Server {
private:
	int _state;
	int _epollFD;
	ConfigParser _configParser;
	std::map<int, Socket *> _sockets;
	std::map<int, Client *> _clients;

	void setState(int state);
	void setEpollFD(int epollFD);
	void checkTimeouts(time_t currentTime);
	void handleEvent(epoll_event *events, int i);
	void handleClientConnection(int fd);
	void handleClientDisconnection(int fd);

public:
	Server();
	~Server();

	void init();
	void run();
	void stop();
	int getState() const;
	int getEpollFD() const;
	ConfigParser &getConfigParser();
	std::map<int, Socket *> getSockets() const;
	Socket *getSocket(int fd);
	std::map<int, Client *> getClients() const;
	Client *getClient(int fd);
};

#endif
