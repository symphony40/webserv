#include "Request.hpp"
#include "main.hpp"

std::string	Request::getParseStateString(parsingState state) {
	switch (state) {
		case BODY_END: return "BODY_END";
		case BODY_INIT: return "BODY_INIT";
		case BODY_PROCESS: return "BODY_PROCESS";
		case CGI_END: return "CGI_END";
		case CGI_INIT: return "CGI_INIT";
		case CGI_PROCESS: return "CGI_PROCESS";
		case FINISH: return "FINISH";
		case HEADERS_END: return "HEADERS_END";
		case HEADERS_INIT: return "HEADERS_INIT";
		case HEADERS_PARSE_END: return "HEADERS_PARSE_END";
		case HEADERS_PARSE_KEY: return "HEADERS_PARSE_KEY";
		case HEADERS_PARSE_VALUE: return "HEADERS_PARSE_VALUE";
		case INIT: return "INIT";
		case REQUEST_LINE_END: return "REQUEST_LINE_END";
		case REQUEST_LINE_HTTP_VERSION: return "REQUEST_LINE_HTTP_VERSION";
		case REQUEST_LINE_METHOD: return "REQUEST_LINE_METHOD";
		case REQUEST_LINE_URI: return "REQUEST_LINE_URI";
		default: return "UNKNOWN";
	}
}

Request::Request(Client *client) : _client(client), _serverBlock(NULL), _locationBlock(NULL),  _rawRequest(""), _httpMethod(""), _uri(""), _path(""), _httpVersion(""), _isChunked(false), _cgi(this), _contentLength(0),  _chunkSize(-1), _timeout(0), _state(Request::INIT), _stateCode(REQUEST_DEFAULT_STATE_CODE) {
	initServer();
}

Request::Request(Request const &obj) { *this = obj; }

Request::~Request() {}

Request &Request::operator=(Request const &obj) {
	if (this != &obj) 	{
		_body = obj._body;
		_chunkSize = obj._chunkSize;
		_client = obj._client;
		_contentLength = obj._contentLength;
		_headers = obj._headers;
		_httpVersion = obj._httpVersion;
		_isChunked = obj._isChunked;
		_locationBlock = obj._locationBlock;
		_httpMethod = obj._httpMethod;
		_path = obj._path;
		_query = obj._query;
		_rawRequest = obj._rawRequest;
		_serverBlock = obj._serverBlock;
		_state = obj._state;
		_stateCode = obj._stateCode;
		_uri = obj._uri;
	}
	return *this;
}


void Request::parse(std::string const &rawRequest) {
	if (_state == Request::FINISH) {
		return;
	}
	if (_state == Request::INIT) {
		Logger::log(Logger::TRACE, "%.*s", 80, rawRequest.substr(0, rawRequest.find("\n")).c_str());
		initTimeout();
	}
	if (rawRequest.empty()) {
		Logger::log(Logger::WARNING, "Empty request");
		return;
	}
	_rawRequest += rawRequest;
	Logger::log(Logger::DEBUG, "Parsing request: %s", _rawRequest.c_str());
	parseRequestLine();
	parseHeaders();
	parseBody();
}

void Request::parseRequestLine() {
	if (_state > Request::REQUEST_LINE_END) {
		return Logger::log(Logger::DEBUG, "Request line already parsed");
	}
	if (_state == Request::INIT) {
		setState(Request::REQUEST_LINE_METHOD);
	}
	if (_state == Request::REQUEST_LINE_METHOD) {
		parseMethod();
	}
	if (_state == Request::REQUEST_LINE_URI) {
		parseUri();
	}
	if (_state == Request::REQUEST_LINE_HTTP_VERSION) {
		parseHttpVersion();
	}
	if (_state == Request::REQUEST_LINE_END) {
		_rawRequest.erase(0, _rawRequest.find_first_not_of(" \t"));
		if (_rawRequest.empty()) {
			return;
		}
		if (_rawRequest[0] == '\n') {
			_rawRequest.erase(0, 1);
			return setState(Request::HEADERS_INIT);
		}
		if (_rawRequest[0] == '\r') {
			if (_rawRequest.size() < 2) {
				return;
			}
			if (_rawRequest[1] == '\n') {
				_rawRequest.erase(0, 2);
				return setState(Request::HEADERS_INIT);
			}
			return setError(400);
		}
		return setError(400);
	}	
}

void Request::parseMethod() {
	size_t i = -1;
	size_t rawSize = _rawRequest.size();
	bool found = false;
	while (++i < rawSize) {
		if (_rawRequest[i] == ' ') {
			found = true;
			break;
		}
		if (!std::isalpha(_rawRequest[i])) {
			return setError(400);
		}
		_httpMethod += _rawRequest[i];
	}
	_rawRequest.erase(0, found ? i + 1 : i);
	if (found) {
		if (_httpMethod.empty()) {
			return setError(400);
		}
		if (!ConfigParser::isMethodSupported(_httpMethod)) {
			return setError(405);
		}
		Logger::log(Logger::DEBUG, "Method: %s", _httpMethod.c_str());
		setState(Request::REQUEST_LINE_URI);
	}
}

void Request::parseUri() {
	size_t i = -1;
	size_t rawSize = _rawRequest.size();
	bool found = false;
	while (++i < rawSize) {
		if (_uri.empty() && (_rawRequest[i] == ' ' || _rawRequest[i] == '\t')) {
			continue;
		}
		if (_rawRequest[i] == ' ') {
			found = true;
			break;
		}
		if (!std::isprint(_rawRequest[i])) {
			return setError(400);
		}
		_uri += _rawRequest[i];
	}
	_rawRequest.erase(0, found ? i + 1 : i);
	if (found) {
		if (_uri.empty()) {
			return setError(400);
		}
		if (processUri() == FAIL) {
			return;
		}
		Logger::log(Logger::DEBUG, "URI: %s", _uri.c_str());
		return setState(Request::REQUEST_LINE_HTTP_VERSION);
	}
}

void Request::parseHttpVersion() {
	size_t i = -1;
	size_t rawSize = _rawRequest.size();
	bool found = false;
	while (++i < rawSize) {
		if (_httpVersion.empty() && (_rawRequest[i] == ' ' || _rawRequest[i] == '\t')) {
			continue;
		}
		if (_rawRequest[i] != 'H' && _rawRequest[i] != 'T' && _rawRequest[i] != 'P' && _rawRequest[i] != '/' && _rawRequest[i] != '.' && !std::isdigit(_rawRequest[i])) {
			found = true;
			break;
		}
		_httpVersion += _rawRequest[i];
	}
	_rawRequest.erase(0, i);
	if (found) {
		if (_httpVersion.empty()) {
			return setError(400);
		}
		if (!ConfigParser::isHttpVersionSupported(_httpVersion)) {
			return setError(505);
		}
		setState(Request::REQUEST_LINE_END);
	}
}

void Request::parseHeaders() {
	if (_state < Request::HEADERS_INIT) {
		return Logger::log(Logger::DEBUG, "Request line not parsed yet");
	}
	if (_state > Request::HEADERS_END) {
		return Logger::log(Logger::DEBUG, "Headers already parsed");
	}
	if (_state == Request::HEADERS_INIT) {
		setState(Request::HEADERS_PARSE_KEY);
	}
	if (_state == Request::HEADERS_PARSE_KEY) {
		parseHeadersKey();
	}
	if (_state == Request::HEADERS_PARSE_VALUE) {
		parseHeadersValue();
	}
	if (_state == Request::HEADERS_PARSE_END) {
		_rawRequest.erase(0, _rawRequest.find_first_not_of(" \t"));
		if (_rawRequest.empty()) {
			return;
		}
		if (_rawRequest[0] == '\n') {
			_rawRequest.erase(0, 1);
			setState(Request::HEADERS_PARSE_KEY);
			return parseHeaders();
		}
		if (_rawRequest[0] == '\r') {
			if (_rawRequest.size() < 2) {
				return;
			}
			if (_rawRequest[1] == '\n') {
				_rawRequest.erase(0, 2);
				setState(Request::HEADERS_PARSE_KEY);
				return parseHeaders();
			}
			return setError(400);
		}
		return setError(400);
	}
}

void Request::parseHeadersKey() {
	if (!_rawRequest.empty() && (_rawRequest[0] == '\r' || _rawRequest[0] == '\n')) {
		if (_rawRequest[0] == '\n') {
			_rawRequest.erase(0, 1);
			if (!_tmpHeaderKey.empty()) {
				return setError(400);
			}
			return setState(Request::BODY_INIT);
		}
		if (_rawRequest.size() < 2) {
			return;
		}
		if (_rawRequest[1] == '\n') {
			_rawRequest.erase(0, 2);
			if (!_tmpHeaderKey.empty()) {
				return setError(400);
			}
			return setState(Request::BODY_INIT);
		}
		return setError(400);
	}
	size_t i = -1;
	size_t rawSize = _rawRequest.size();
	bool found = false;
	while (++i < rawSize) {
		if (_rawRequest[i] == ' ' || _rawRequest[i] == '\t') {
			return setError(400);
		}
		if (_rawRequest[i] == ':') {
			found = true;
			break;
		}
		if (!std::isalnum(_rawRequest[i]) && _rawRequest[i] != '-' && _rawRequest[i] != '_') {
			return setError(400);
		}
		_tmpHeaderKey += _rawRequest[i];
	}
	_rawRequest.erase(0, found ? i + 1 : i);
	if (found) {
		if (_tmpHeaderKey.empty()) {
			return setError(400);
		}
		_tmpHeaderKey.erase(std::remove_if(_tmpHeaderKey.begin(), _tmpHeaderKey.end(), ::isspace), _tmpHeaderKey.end());
		Logger::log(Logger::DEBUG, "Header key: %s", _tmpHeaderKey.c_str());
		setState(Request::HEADERS_PARSE_VALUE);
	}
}

void Request::parseHeadersValue() {
	size_t i = -1;
	size_t rawSize = _rawRequest.size();
	bool found = false;
	while (++i < rawSize) {
		if (_tmpHeaderValue.empty() && (_rawRequest[i] == ' ' || _rawRequest[i] == '\t')) {
			continue;
		}
		if (!std::isprint(_rawRequest[i])) {
			found = true;
			break;
		}
		_tmpHeaderValue += _rawRequest[i];
	}
	_rawRequest.erase(0, i);
	if (found) {
		if (_tmpHeaderValue.empty()) {
			return setError(400);
		}
		if (_headers.find(_tmpHeaderKey) != _headers.end()) {
			return setError(400);
		}
		Logger::log(Logger::DEBUG, "Header value: %s", _tmpHeaderValue.c_str());
		_headers[_tmpHeaderKey] = _tmpHeaderValue;
		_tmpHeaderKey.clear();
		_tmpHeaderValue.clear();
		setState(Request::HEADERS_PARSE_END);
	}
}


void Request::parseBody() {
	if (_state < Request::BODY_INIT) {
		return Logger::log(Logger::DEBUG, "Headers not parsed yet");
	}
	if (_state > Request::BODY_INIT) {
		return Logger::log(Logger::DEBUG, "Body already parsed");
	}
	if (isChunked()) {
		return parseChunkedBody();
	}
	if (_body.writeData(_rawRequest) == FAIL) {
		return setError(500);
	}
	_rawRequest.clear();
	if (_body._size > _serverBlock->getClientMaxBodySize()) {
		return setError(413);
	}
	if (_body._size == _contentLength) {
		return setState(Request::BODY_END);
	}
}

void Request::parseChunkedBody() {
	while (!_rawRequest.empty()) {
		if (_chunkSize == -1) {
			size_t pos = _rawRequest.find("\r\n");
			if (pos == std::string::npos) {
				return;
			}
			std::string line = _rawRequest.substr(0, pos);
			std::istringstream iss(line);
			if (!(iss >> std::hex >> _chunkSize)) {
				setError(400);
				return Logger::log(Logger::ERROR, "[parseChunkedBody] Error parsing chunk size");
			}
			_rawRequest.erase(0, pos + 2);
			if (_chunkSize == 0) {
				return setState(Request::BODY_END);
			}
			Logger::log(Logger::DEBUG, "[parseChunkedBody] Chunk size: %d", _chunkSize);
		}
		size_t pos = _rawRequest.find("\r\n");
		if (pos == std::string::npos) {
			return;
		}
		if (pos != (size_t)_chunkSize) {
			setError(400);
			return Logger::log(Logger::ERROR, "[parseChunkedBody] Chunk size does not match");
		}
		if (_body.writeData(_rawRequest.substr(0, _chunkSize)) == -1) {
			return setError(500);
		}
		_rawRequest.erase(0, _chunkSize + 2);
		_chunkSize = -1;
		if (_body._size > (u_int64_t)_serverBlock->getClientMaxBodySize()) {
			return setError(413);
		}
	}
}

void Request::setState(parsingState state) {
	if (_state == Request::FINISH) {
		return Logger::log(Logger::DEBUG, "[setState] Request already finished");
	}
	if (_state == state) {
		return Logger::log(Logger::DEBUG, "[setState] Request already in this state");
	}
	Logger::log(Logger::DEBUG, "[setState] Request state changed from %s to %s with state code: %d", getParseStateString(_state).c_str(), getParseStateString(state).c_str(), _stateCode);
	_state = state;
	if (_state == Request::BODY_INIT) {
		setHeaderState();
		if (_state == Request::FINISH) {
			return;
		}
		if (_httpMethod != "POST" && _httpMethod != "PUT") {
			return(_cgi._isCGI ? setState(Request::CGI_INIT) : setState(Request::FINISH));
		}
		setTimeout(REQUEST_DEFAULT_BODY_TIMEOUT);
		defineBodyDestination();
	} else if (_state == Request::BODY_END) {
		if (_cgi._isCGI) {
			setState(Request::CGI_INIT);
		} else {
			setState(Request::FINISH);
		}
	} else if (_state == Request::CGI_INIT) {
		Utils::socketEpollModify(g_server->getEpollFD(), _client->getFd(), RESPONSE_FLAGS);
		setTimeout(REQUEST_DEFAULT_CGI_TIMEOUT);
		_cgi.start();
	} else if (_state == Request::FINISH) {
		_timeout = 0;
		Utils::socketEpollModify(g_server->getEpollFD(), _client->getFd(), RESPONSE_FLAGS);
	}
}

void Request::setHeaderState() {
	if (findServer() == FAIL 
		|| findLocation() == FAIL 
		|| checkHttpMethod() == FAIL 
		|| checkTransferEncoding() == FAIL 
		|| checkClientMaxBodySize() == FAIL 
		|| checkCgi() == FAIL) {
		return;
	}
}

void Request::setError(int code) {
	_stateCode = code;
	setState(Request::FINISH);
}

int Request::processUri() {
	if (Utils::urlDecode(_uri) == FAIL) {
		return (setError(400), FAIL);
	}
	size_t pos = _uri.find('?');
	if (pos != std::string::npos) {
		_path = _uri.substr(0, pos);
		_query = _uri.substr(pos + 1);
	} else {
		_path = _uri;
	}
	return OK;
}

int	Request::findServer() {
	if (!_client) {
		Logger::log(Logger::ERROR, "[findServer] Client is NULL");
		setError(500);
		return FAIL;
	}
	std::string host = _headers["Host"];
	if (host.empty()) {
		Logger::log(Logger::ERROR, "[findServer] Host not found in headers");
		setError(400);
		return FAIL;
	}
	Logger::log(Logger::DEBUG, "[findServer] Host: %s", host.c_str());
	Socket *socket = _client->getSocket();
	if (!socket) {
		Logger::log(Logger::ERROR, "[findServer] Socket is NULL");
		setError(500);
		return FAIL;
	}
	std::vector<BlockConfigServer>* servers = socket->getServers();
	for (std::vector<BlockConfigServer>::iterator it = servers->begin(); it != servers->end(); it++) {
		std::vector<std::string> serverNames = it->getServerNames();
		for (std::vector<std::string>::iterator it2 = serverNames.begin(); it2 != serverNames.end(); it2++) {
			if (*it2 == host.substr(0, host.find(":"))) {
				_serverBlock = &(*it);
				break ;
			}
		}
	}
	return 1;
}

int	Request::findLocation() {
	if (!_serverBlock) {
		Logger::log(Logger::ERROR, "[findLocation] Server is NULL");
		setError(500);
		return FAIL;
	}
	BlockConfigRoute *locationBlock = NULL;
	std::vector<BlockConfigRoute>* locations = _serverBlock->getLocations();
	int lastClosestMatch = -1;
	for (std::vector<BlockConfigRoute>::iterator it = locations->begin(); it != locations->end(); ++it) {
		std::string	path = it->getPath();
		if (checkPathsMatch(_path, path)) {
			if ((int)path.size() > lastClosestMatch) {
				lastClosestMatch = path.size();
				locationBlock = &(*it);
			}
		}
	}
	_locationBlock = locationBlock;
	return OK;
}

int Request::checkTransferEncoding() {
	if (_headers.find("Transfer-Encoding") != _headers.end()) {
		if (_headers["Transfer-Encoding"] == "chunked") {
			_isChunked = true;
		} else if (_headers["Transfer-Encoding"] != "identity")	{
			Logger::log(Logger::ERROR, "[checkTransferEncoding] Transfer-Encoding not supported: %s", _headers["Transfer-Encoding"].c_str());
			setError(501);
			return FAIL;
		}
	}
	return OK;
}

int Request::checkClientMaxBodySize() {
	if (_headers.find("Content-Length") != _headers.end()) {
		std::istringstream iss(_headers["Content-Length"]);
		iss >> _contentLength;
	}
	if (_contentLength > _serverBlock->getClientMaxBodySize()) {
		Logger::log(Logger::ERROR, "[checkClientMaxBodySize] Content-Length too big, max body size: %d, content length: %d", _serverBlock->getClientMaxBodySize(), _contentLength);
		setError(413);
		return FAIL;
	}
	return OK;
}

int	Request::checkHttpMethod() {
	if (!_locationBlock || _locationBlock->isMethodAllowed(BlockConfigRoute::converStrToMethod(_httpMethod))) {
		return OK;
	}
	Logger::log(Logger::ERROR, "[checkHttpMethod] Method not allowed: %s", _httpMethod.c_str());
	setError(405);
	return FAIL;
}

int	Request::checkPathsMatch(std::string const &path, std::string const &parentPath) {
	size_t pathSize = path.size();
	size_t parentPathSize = parentPath.size();
	if (path.compare(0, parentPathSize, parentPath) == 0) {
		if (pathSize== parentPathSize || path[parentPathSize] == '/' || parentPath == "/") {
			return FAIL;
		}
	}
	return OK;
}

int Request::checkCgi() {
	if (!_locationBlock) {
		return OK;
	}
	std::vector<std::string> allPathsLocations = getAllPathLocations();
	for (size_t i = 0; i < allPathsLocations.size(); i++) {
		for (std::map<std::string, std::string>::const_iterator it = _locationBlock->getCGI().begin(); it != _locationBlock->getCGI().end(); it++) {
			if (Utils::getExtension(allPathsLocations[i]) == it->first) {
				if (Utils::fileExists(allPathsLocations[i])) {
					return (setCgi(true, allPathsLocations[i], it->second), OK);
				}
			}
		}
	}
	return OK;
}

void Request::defineBodyDestination() {
	if (!_cgi._isCGI && (_httpMethod == "POST" || _httpMethod == "PUT")) {
		if (_headers.find("Content-Type") != _headers.end() && _headers["Content-Type"] == "multipart/form-data") {
			return setError(415);
		}
		bool isPathDir = _path.size() > 1 && _path[_path.size() - 1] == '/';
		_body._path = (_locationBlock && !_locationBlock->getRoot().empty()) ? _locationBlock->getRoot() : _serverBlock->getRoot();
		if (!isPathDir) {
			_body._path += _path;
			if (_httpMethod == "POST" && Utils::fileExists(_body._path)) {
				setError(403);
			}
			_body._fd = open(_body._path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if (_body._fd == FAIL) {
				setError(403);
			}
		} else {
			_body._path += _path;
			if (_headers.find("Filename") != _headers.end()) {
				_body._path += "/" + _headers["Filename"];
				if (_httpMethod == "POST" && Utils::fileExists(_body._path)) {
					setError(403);
				}
				_body._fd = open(_body._path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
				if (_body._fd == FAIL) {
					setError(403);
				}
			} else {
				_body._path += "/upload_";
				if (Utils::createFileRandomSuffix(_body._path, _body._fd) == FAIL) {
					setError(403);
				}
			}
		}
		_body._isTemp = false;
	} else {
		if (Utils::createTempFile(_body._path, _body._fd) == FAIL) {
			setError(500);
		}
		_body._isTemp = true;
	}
}

void Request::initTimeout() {
	time_t currentTime = time(NULL);
	_timeout = currentTime + REQUEST_DEFAULT_HEADER_TIMEOUT;
}

void Request::checkTimeout() {
	if (_timeout == 0 || _state == Request::FINISH) {
		return;
	}
	time_t currentTime = time(NULL);
	if (currentTime > _timeout) {
		Logger::log(Logger::ERROR, "[checkTimeout] Client %d timeout", _client->getFd());
		if (_state >= Request::CGI_INIT) {
			_cgi.killCgiProcess();
			setError(504);
		} else {
			setError(408);
		}
	}
}

void Request::initServer() {
	if (!_client) {
		Logger::log(Logger::ERROR, "[initServer] No client found");
		setError(500);
		return ;
	}
	Socket *socket = _client->getSocket();
	if (!socket) {
		Logger::log(Logger::ERROR, "[initServer] No socket found");
		setError(500);
		return ;
	}
	std::vector<BlockConfigServer>* servers = socket->getServers();
	if (servers->empty()) {
		Logger::log(Logger::ERROR, "[initServer] No server found");
		setError(500);
		return ;
	}
	_serverBlock = &servers->front();
}

std::vector<std::string> Request::getAllPathLocations() {
	if (!_locationBlock) {
		return std::vector<std::string>();
	}
	std::vector<std::string> allPathsLocations;
	std::string path = _path;
	std::string root = _locationBlock->getRoot();
	std::string alias = _locationBlock->getAlias();
	std::vector<std::string> indexes = _locationBlock->getIndexes();
	bool isAlias = false;
	if (root.empty()) {
		root = _serverBlock->getRoot();
	}
	if (!alias.empty()) {
		root = alias;
		isAlias = true;
	}
	if (path[path.size() - 1] != '/') {
		if (isAlias) {
			path = path.substr(_locationBlock->getPath().size());
		}
		allPathsLocations.push_back(root + path);
	}
	for (size_t i = 0; i < indexes.size(); i++) {
		std::string index = indexes[i];
		std::string tmpPath = path;
		if (path == "/") {
			path = root + "/" + index;
		} else if (isAlias) {
			path = root + "/" + index;
		} else {
			path = root + path + index;
		}
		allPathsLocations.push_back(path);
		path = tmpPath;
	}
	return allPathsLocations;
}


BlockConfigRoute *Request::getLocation() const { return _locationBlock; }

BlockConfigServer *Request::getServer() const { return _serverBlock; }

bool Request::isCgi() const { return _cgi._isCGI; }

bool Request::isChunked() const { return _isChunked; }

Client *Request::getClient() const { return _client; }

int Request::getChunkSize() const { return _chunkSize; }

int Request::getStateCode() const { return _stateCode; }

Request::parsingState Request::getState() const { return _state; }

RequestBody &Request::getBody() { return _body; }

RequestCgi &Request::getCgi() { return _cgi; }

std::map<std::string, std::string> Request::getHeaders() const { return _headers; }

std::string Request::getRawRequest() const { return _rawRequest; }

std::string Request::getHttpVersion() const { return _httpVersion; }

std::string Request::getMethod() const { return _httpMethod; }

std::string Request::getPath() const { return _path; }

std::string Request::getQuery() const { return _query; }

std::string Request::getUri() const { return _uri; }

time_t Request::getTimeout() const { return _timeout; }

unsigned long long Request::getContentLength() const { return _contentLength; }

void Request::setTimeout(int timeout) { _timeout = time(NULL) + timeout; }

void Request::setTimeout(time_t timeout) { _timeout = timeout; }

void Request::setStateCode(int code) { _stateCode = code; }
