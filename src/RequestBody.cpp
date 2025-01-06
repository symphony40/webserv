#include "RequestBody.hpp"

RequestBody::RequestBody() : _fd(FAIL), _isTemp(false), _size(0) {}

RequestBody::RequestBody(bool isTemp) : _fd(FAIL), _isTemp(isTemp), _size(0) {}

RequestBody::RequestBody(RequestBody const &obj) { *this = obj; }

RequestBody::~RequestBody() {
	if (_fd != FAIL) {
		Utils::protectedCall(close(_fd), "failed to close file", false);
	}
	if (_path.size() && _isTemp) {
		remove(_path.c_str());
	}
}

RequestBody &RequestBody::operator=(RequestBody const &obj) {
	if (this != &obj) {
		_path = obj._path;
		_fd = obj._fd;
		_isTemp = obj._isTemp;
		_size = obj._size;
	}
	return *this;
}


int	RequestBody::writeData(std::string const &data) {
	if (_fd == FAIL) {
		Logger::log(Logger::ERROR, "[RequestBody::writeData] File descriptor not set");
		return FAIL;
	}
	if (write(_fd, data.c_str(), data.size()) == FAIL) {
		Logger::log(Logger::ERROR, "[RequestBody::writeData] Error writing in file");
		return FAIL;
	}
	_size += data.size();
	return OK;
}

std::string RequestBody::getPath() const { return _path; }

int RequestBody::getFd() const { return _fd; }

bool RequestBody::isTemp() const { return _isTemp; }

unsigned long long RequestBody::getSize() const { return _size; }