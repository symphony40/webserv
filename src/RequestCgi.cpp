#include "RequestCgi.hpp"

RequestCgi::RequestCgi() : _request(NULL), _isCGI(false), _path(""), _execPath(""), _cgiHandler(NULL) {}

RequestCgi::RequestCgi(Request* request) : _request(request), _isCGI(false), _path(""), _execPath(""), _cgiHandler(NULL) {}

RequestCgi::RequestCgi(RequestCgi const &obj) {	*this = obj; }

RequestCgi::~RequestCgi() {
	if (_cgiHandler) {
		delete _cgiHandler;
	}
}

RequestCgi &RequestCgi::operator=(RequestCgi const &obj) {
	if (this != &obj) {
		_request = obj._request;
		_isCGI = obj._isCGI;
		_path = obj._path;
		_execPath = obj._execPath;
		_cgiHandler = obj._cgiHandler;
	}
	return *this;
}

void RequestCgi::_start() {
	try {
		if (_cgiHandler) {
			delete _cgiHandler;
		}
		_cgiHandler = new CgiExecutor(this);
		if (_cgiHandler == NULL) {
			throw std::bad_alloc();
		}
		_cgiHandler->init();
		_cgiHandler->execute();
	} catch (ChildProcessException &e) {
		throw ChildProcessException();
	} catch (IntException &e) {
		return (_request->setError(e.code()));
	} catch (std::exception &e) {
		Logger::log(Logger::ERROR, "Failed to handle CGI: %s", e.what());
		return (_request->setError(500));
	}
}

void RequestCgi::_checkState() {
	if (_request->_state == Request::FINISH || _cgiHandler == NULL) {
		return;
	}
	try {
		int status;
		pid_t wpid = waitpid(_cgiHandler->_pid, &status, WNOHANG);
		if (wpid == FAIL) {
			throw IntException(500);
		}
		if (!wpid) {
			return;
		}
		if (WIFEXITED(status) && !WEXITSTATUS(status)) {
			_request->setState(Request::FINISH);
		} else {
			Logger::log(Logger::ERROR, "[RequestCgi] CGI process exited abnormally");
			_request->setError(502);
		}
	} catch (IntException &e) {
		Logger::log(Logger::DEBUG, "[RequestCgi] CGI process failed: %d", e.code());
		return _request->setError(e.code());
	}
}

void RequestCgi::_kill() {
	if (_cgiHandler == NULL) {
		return;
	}
	kill(_cgiHandler->_pid, SIGTERM); // OR SIGKILL
}
