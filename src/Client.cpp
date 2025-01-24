#include "Client.hpp"

Client::Client(int fd, Socket *socket) : _fd(fd), _socket(socket), _request(new Request(this)), _response(new Response(this)), _lastActivityAt(time(NULL)) {}

Client::~Client() {
	if (_fd != -1) {
		Utils::tryCall(close(_fd), "[~Client] Failed to close client socket", false);
	}
	if (_request) {
		delete _request;
	}
	if (_response) {
		delete _response;
	}
}

void Client::handleRequest() {
	Logger::log(Logger::DEBUG, "[handleRequest] Handling request from client %d", _fd);
	char buffer[CLIENT_READ_BUFFER_SIZE + 1];
	int bytesRead = 0;

	memset(buffer, 0, CLIENT_READ_BUFFER_SIZE + 1);
	bytesRead  = recv(_fd, buffer, CLIENT_READ_BUFFER_SIZE, 0);
	if (bytesRead > 0) {
		Logger::log(Logger::DEBUG, "[handleRequest] Received %d bytes from client %d", bytesRead, _fd);
		buffer[bytesRead] = '\0';
	} else if (bytesRead < 0) {
		throw std::runtime_error("Error with recv function");
	} else if (!bytesRead) {
		throw Client::DisconnectedException();
	}
	
	if (_request->getState() == Request::FINISH) {
		return (Logger::log(Logger::DEBUG, "[handleRequest] Request already finished"));
	}

	std::string str(buffer, bytesRead);
	_request->parse(str);
}

void Client::handleResponse(int epollFD) {
	if (_response->generateResponse() == -1) {
		return;
	}
	Logger::log(Logger::DEBUG, "Response to send: \n%s", _response->getResponse().c_str());
	
	int bytesSent = -1;
	if (getFd() != -1) {
		bytesSent = send(getFd(), _response->getResponse().c_str(), _response->getResponseSize(), 0);
	}
	
	if (bytesSent < 0) {
		throw std::runtime_error("Error with send function");
	} else {
		Logger::log(Logger::DEBUG, "Sent %d bytes to client %d", bytesSent, getFd());
	}

	if (getResponse()->getState() == Response::FINISH) {
		if (_request->getStateCode() != Request::FINISH) {
			throw Client::DisconnectedException();
		}
		Logger::log(Logger::DEBUG, "Response sent to client %d", getFd());
		reset();
		Utils::socketEpollModify(epollFD, getFd(), REQUEST_FLAGS);
	}
}

void Client::reset() {
	delete _request;
	_request = new Request(this);
	delete _response;
	_response = new Response(this);
}

void Client::checkCgi() {
	if (!_request || !_request->_cgi._isCGI) {
		return;
	}
	return _request->_cgi.checkState();
}


int Client::getFd() const { return _fd; }

Request *Client::getRequest() const { return _request; }

Socket *Client::getSocket() const { return _socket; }

Response *Client::getResponse() const { return _response; }

time_t Client::getTimeOfLastActivity() const { return _lastActivityAt; }

void Client::updateTimeOfLastActivity() { _lastActivityAt = time(NULL); }
