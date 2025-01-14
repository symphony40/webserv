#include "Server.hpp"

Server::Server() : _state(SERVER_STATE_INIT), _epollFD(-1) {}

Server::~Server() {
	if (_epollFD != -1) {
		Utils::tryCall(close(_epollFD), "Failed to close epoll instance", false);
	}
	for (std::map<int, Socket *>::iterator it = _sockets.begin(); it != _sockets.end(); it++) {
		delete it->second;
	}
	_sockets.clear();
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); it++) {
		delete it->second;
	}
	_clients.clear();
}

void Server::stop() { setState(SERVER_STATE_STOP); }

void Server::init() {
	Logger::log(Logger::DEBUG, "[Server::init] Create epoll instance...");
	setEpollFD(Utils::tryCall(epoll_create1(O_CLOEXEC), "Failed to create epoll instance"));
	Logger::log(Logger::DEBUG, "#------------------------------#");
	Logger::log(Logger::DEBUG, "|  Create listening sockets... |");
	Logger::log(Logger::DEBUG, "#------------------------------#");
	std::map<std::string, std::vector<BlockConfigServer> > &servers = _configParser.getServers();
	for (std::map<std::string, std::vector<BlockConfigServer> >::iterator it = servers.begin(); it != servers.end(); it++) {
		int socketFD = Utils::tryCall(socket(AF_INET, SOCK_STREAM, 0), "Error with socket function");
		_sockets[socketFD] = new Socket(socketFD, Utils::extractAddress(it->first), Utils::extractPort(it->first), &it->second);
		Utils::socketEpollAdd(_epollFD, socketFD, REQUEST_FLAGS);
	}
	setState(SERVER_STATE_READY);
}

void Server::handleClientConnection(int fd) {
	Logger::log(Logger::DEBUG, "[Server::handleClientConnection] New client connected on file descriptor %d", fd);
	struct sockaddr_in addr;
	socklen_t addrLen = sizeof(addr);
	int clientFD = Utils::tryCall(accept(fd, (struct sockaddr *)&addr, &addrLen), "Error with accept function");
	_clients[clientFD] = new Client(clientFD, _sockets[fd]);
	Utils::tryCall(fcntl(clientFD, F_SETFL, O_NONBLOCK), "Error with fcntl function");
	Utils::socketEpollAdd(_epollFD, clientFD, REQUEST_FLAGS);
}

void Server::handleClientDisconnection(int fd) {
	Logger::log(Logger::DEBUG, "[Server::handleClientDisconnection] Client disconnected on file descriptor %d", fd);
	Utils::socketEpollDelete(_epollFD, fd);
	std::map<int, Client *>::iterator it = _clients.find(fd);
	if (it != _clients.end()) {
		delete it->second;
		_clients.erase(it);
	}
}

void Server::handleEvent(epoll_event *events, int i) {
	uint32_t event = events[i].events;
	int fd = events[i].data.fd;
	try {
		if (event & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)) {
			throw Client::DisconnectedException();
		}
		if (event & EPOLLIN) {
			if (_clients.find(fd) == _clients.end()) {
				handleClientConnection(fd);
			} else {
				_clients[fd]->updateTimeOfLastActivity();
				_clients[fd]->handleRequest();
			}
		}
		if (event & EPOLLOUT) {
			_clients[fd]->updateTimeOfLastActivity();
			_clients[fd]->checkCgi();
			if (_clients[fd]->getRequest() && _clients[fd]->getRequest()->getState() == Request::FINISH)
				_clients[fd]->handleResponse(_epollFD);
		}
	} catch (ChildProcessException &e) {
		throw ChildProcessException();
	} catch (Client::DisconnectedException &e) {
		handleClientDisconnection(fd);
	} catch (std::exception const &e) {
		Logger::log(Logger::ERROR, "[Server::handleEvent] Error with client %d : %s", fd, e.what());
		handleClientDisconnection(fd);
	}
}

void Server::checkTimeouts(time_t currentTime) {
	std::map<int, Client *>::iterator it = _clients.begin();
	while (it != _clients.end()) {
		it->second->getRequest()->checkTimeout();
		if (currentTime - it->second->getTimeOfLastActivity() > INACTIVITY_TIMEOUT) {
			Logger::log(Logger::DEBUG, "[Server::checkTimeouts] Client %d timed out", it->first);
			Utils::socketEpollDelete(_epollFD, it->first);
			delete it->second;
			_clients.erase(it++);
		} else {
			it++;
		}
	}
}

void Server::run() {
	if (getState() != SERVER_STATE_READY) {
		Logger::log(Logger::FATAL, "Server is not ready to run");
	}
	setState(SERVER_STATE_RUN);
	time_t lastTimeoutCheck = time(NULL);
	epoll_event	events[MAX_EVENTS];
	while (getState() == SERVER_STATE_RUN) {
		static int lastValueCheck = INT8_MIN;
		int nfds = Utils::tryCall(epoll_wait(_epollFD, events, MAX_EVENTS, SERVER_DEFAULT_EPOLL_WAIT), "Error with epoll_wait function");
		if (lastValueCheck != nfds) {
			Logger::log(Logger::DEBUG, "[Server::run] There are %d file descriptors ready for I/O after epoll wait", nfds);
		}
		lastValueCheck = nfds;	
		for (int i = 0; i < nfds; i++) {
			handleEvent(events, i);
		}
		time_t currentTime = time(NULL);
		if (currentTime - lastTimeoutCheck >= TIMEOUT_CHECK_INTERVAL) {
			checkTimeouts(currentTime);
			lastTimeoutCheck = currentTime;
		}
	}
}

void Server::setState(int state) {
	if (state == SERVER_STATE_INIT) {
		Logger::log(Logger::INFO, "Parsing completed");
	} else if (state == SERVER_STATE_READY) {
		Logger::log(Logger::INFO, "Server is ready to run");
	} else if (state == SERVER_STATE_RUN) {
		Logger::log(Logger::INFO, "Server is running");
	} else if (state == SERVER_STATE_STOP) {
		Logger::log(Logger::INFO, "Server is stopping...");
	}
	_state = state;
}

void Server::setEpollFD(int epollFD) { _epollFD = epollFD; }

int Server::getState() const { return _state; }

int Server::getEpollFD() const { return _epollFD; }

ConfigParser &Server::getConfigParser() { return _configParser; }

std::map<int, Socket *> Server::getSockets() const { return _sockets; }

Socket *Server::getSocket(int fd) { return _sockets[fd]; }

std::map<int, Client *> Server::getClients() const { return _clients; }

Client *Server::getClient(int fd) { return _clients[fd]; }