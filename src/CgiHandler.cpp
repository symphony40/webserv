#include "CgiHandler.hpp"

CgiHandler::CgiHandler(Response *response) : _response(response), _isChunked(false), _state(CgiHandler::INIT) {}

CgiHandler::CgiHandler(const CgiHandler &obj) {	*this = obj; }

CgiHandler::~CgiHandler() {}

CgiHandler &CgiHandler::operator=(const CgiHandler &obj) {
	if (this != &obj) {
		_response = obj._response;
		_output = obj._output;
		_headers = obj._headers;
		_tmpHeaderKey = obj._tmpHeaderKey;
		_tmpHeaderValue = obj._tmpHeaderValue;
		_isChunked = obj._isChunked;
		_state = obj._state;
	}
	return *this;
}

std::string	CgiHandler::getStateStr(cgiHandlerState state) {
	switch (state) {
		case INIT: return "INIT";
		case HEADERS_PARSE_KEY: return "HEADERS_PARSE_KEY";
		case HEADERS_PARSE_VALUE: return "HEADERS_PARSE_VALUE";
		case HEADERS_PARSE_END: return "HEADERS_PARSE_END";
		case HEADERS_END: return "HEADERS_END";
		case BODY: return "BODY";
		case FINISH: return "FINISH";
		default: return "UNKNOWN";
	}
}


void CgiHandler::setState(cgiHandlerState state) {
	if (_state == CgiHandler::FINISH || _state == state) {
		return;
	}
	Logger::log(Logger::DEBUG, "CgiHandler state: %s -> %s", getStateStr(_state).c_str(), getStateStr(state).c_str());
	_state = state;
	if (_state == CgiHandler::BODY) {
		if (_headers.find("content-length") == _headers.end()) {
			_isChunked = true;
			_headers["transfer-encoding"] = "chunked";
		}
		if (checkHeaders() == FAIL) {
			return;
		}
		_response->_response += _response->_request->getHttpVersion() + " " + Utils::intToString(_response->_request->getStateCode()) + " " + Utils::getHttpStatusMessage(_response->_request->getStateCode()) + "\r\n";
		for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++) {
			_response->_response += it->first + ": " + it->second + "\r\n";
		}
		_response->_response += "\r\n";
	} else if (_state == CgiHandler::FINISH) {
		return _response->setState(Response::FINISH);
	}
}

void CgiHandler::parse(const std::string &data) {
	if (_state == CgiHandler::FINISH) {
		return;
	}
	_output += data;
	parseHeaders();
	parseBody();
}

void	CgiHandler::parseHeaders() {
	if (_state < CgiHandler::INIT) {
		return (Logger::log(Logger::DEBUG, "Request line not parsed yet"));
	}
	if (_state > CgiHandler::HEADERS_END) {
		return (Logger::log(Logger::DEBUG, "Headers already parsed"));
	}
	if (_state == CgiHandler::INIT) {
		_state = CgiHandler::HEADERS_PARSE_KEY;
	}
	if (_state == CgiHandler::HEADERS_PARSE_KEY) {
		parseHeadersKey();
	}
	if (_state == CgiHandler::HEADERS_PARSE_VALUE) {
		parseHeadersValue();
	}
	if (_state == CgiHandler::HEADERS_PARSE_END) {
		_output.erase(0, _output.find_first_not_of(" \t"));
		if (_output.empty()) {
			return;
		}
		if (_output[0] == '\n') {
			_output.erase(0, 1);
			setState(CgiHandler::HEADERS_PARSE_KEY);
			return parseHeaders();
		}
		if (_output[0] == '\r') {
			if (_output.size() < 2) {
				return;
			}
			if (_output[1] == '\n') {
				_output.erase(0, 2);
				setState(CgiHandler::HEADERS_PARSE_KEY);
				return parseHeaders();
			}
			return _response->setError(500);
		}
		return _response->setError(500);
	}
}

void CgiHandler::parseHeadersKey() {
	if (!_output.empty() && (_output[0] == '\r' || _output[0] == '\n')) {
		if (_output[0] == '\n') {
			_output.erase(0, 1);
			if (!_tmpHeaderKey.empty()) {
				return _response->setError(500);
			}
			return setState(CgiHandler::BODY);
		}
		if (_output.size() < 2) {
			return;
		}
		if (_output[1] == '\n') {
			_output.erase(0, 2);
			if (!_tmpHeaderKey.empty()) {
				return _response->setError(500);
			}
			return setState(CgiHandler::BODY);
		}
		return _response->setError(500);
	}
	size_t i = -1;
	size_t rawSize = _output.size();
	bool found = false;
	while (++i < rawSize) {
		if (_output[i] == ' ' || _output[i] == '\t') {
			return _response->setError(500);
		}
		if (_output[i] == ':') {
			found = true;
			break;
		}
		if (!std::isalnum(_output[i]) && _output[i] != '-' && _output[i] != '_') {
			return _response->setError(500);
		}
		_tmpHeaderKey += _output[i];
	}
	_output.erase(0, found ? i + 1 : i);
	if (found) {
		if (_tmpHeaderKey.empty()) {
			return _response->setError(500);
		}
		_tmpHeaderKey.erase(std::remove_if(_tmpHeaderKey.begin(), _tmpHeaderKey.end(), ::isspace), _tmpHeaderKey.end());
		Logger::log(Logger::DEBUG, "Header key: %s", _tmpHeaderKey.c_str());
		setState(CgiHandler::HEADERS_PARSE_VALUE);
	}
}

void CgiHandler::parseHeadersValue() {
	size_t i = -1;
	size_t rawSize = _output.size();
	bool found = false;
	while (++i < rawSize) {
		if (_tmpHeaderValue.empty() && (_output[i] == ' ' || _output[i] == '\t')) {
			i++;
			continue;
		}
		if (!std::isprint(_output[i])) {
			found = true;
			break;
		}
		_tmpHeaderValue += _output[i];
	}
	_output.erase(0, i);
	if (found) {
		if (_tmpHeaderValue.empty() || _headers.find(_tmpHeaderKey) != _headers.end()) {
			return _response->setError(500);
		}
		_headers[Utils::toLowerCase(_tmpHeaderKey)] = _tmpHeaderValue;
		_tmpHeaderKey.clear();
		_tmpHeaderValue.clear();
		setState(CgiHandler::HEADERS_PARSE_END);
	}
}

void CgiHandler::parseBody() {
	if (_state < CgiHandler::BODY) {
		return (Logger::log(Logger::DEBUG, "Headers not parsed yet"));
	}
	if (_state > CgiHandler::BODY) {
		return (Logger::log(Logger::DEBUG, "Body already parsed"));
	}
	if (_isChunked) {
		std::string chunkSizeStr = Utils::intToHex(_output.size()) + "\r\n";
		_response->_response += chunkSizeStr + _output + "\r\n";
	} else {
		_response->_response += _output;
	}
	_output.clear();
}

int	CgiHandler::checkHeaders() {
	if (_headers.empty()) {
		return (_response->setError(502), FAIL);
	}
	if (_headers.find("status") != _headers.end()) {
		int statusCode = atoi(_headers["statusCode"].c_str());
		_response->_request->setStateCode(statusCode);
		if (statusCode >= 400) {
			return (_response->setError(statusCode), FAIL);
		}
		_headers.erase("status");
	}
	if (_headers.find("content-type") == _headers.end()) {
		return (_response->setError(502), FAIL);
	}
	return OK;
}

std::string CgiHandler::getOutput() const { return _output; }

CgiHandler::cgiHandlerState CgiHandler::getState() const { return _state; }
