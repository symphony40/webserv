#include "Response.hpp"

Response::Response(Client *client) : _request(client->getRequest()), _cgiHandler(this), _state(Response::INIT), _fileFd(FAIL) {}

Response::~Response() {
	if (_fileFd != FAIL) {
		close(_fileFd);
	}
}

bool Response::isRedirect() {
	std::string path = _request->getPath();
	std::string root;
	bool isLoc;

	isLoc = !_request->getRoute() ? false : true;
	if (isLoc && !_request->getRoute()->getRewrite().second.empty()) {
		std::pair<int, std::string> rewrite = _request->getRoute()->getRewrite();
		_response = "HTTP/1.1 " + Utils::intToString(rewrite.first) + " " + Utils::getHttpStatusMessage(rewrite.first) + "\r\n";
		_response += "Location:" + rewrite.second + "\r\n";
		_response += "Content-Length: 0\r\n";
		_response += "\r\n";
		return true;
	}
	if (isLoc && !_request->getRoute()->getRoot().empty()) {
		root = _request->getRoute()->getRoot();
	} else {
		root = _request->getServer()->getRoot();
	}
	if (path[path.size() - 1] == '/' || path == "/") {
		return false;
	}
	if (Utils::directoryExist((root + path).c_str()) || (isLoc && Utils::directoryExist((_request->getRoute()->getAlias() + path.substr(_request->getRoute()->getPath().size())).c_str()))) {
		std::string host = _request->getHeaders()["Host"];
		_response = "HTTP/1.1 301 Moved Permanently\r\n";
		_response += "Location: http://" + host + path + "/\r\n";
		_response += "Content-Length: 0\r\n";
		_response += "\r\n";
		Logger::log(Logger::DEBUG, "REDIRECT");
		return true;
	}
	Logger::log(Logger::DEBUG, "NO REDIRECT");
	return false;
}

std::vector<std::string> Response::getAllPathsInRoute() {
	std::vector<std::string> allPaths;
	std::string path = _request->getPath();
	std::string root = _request->getRoute()->getRoot();
	std::string alias = _request->getRoute()->getAlias();
	std::vector<std::string> indexes = _request->getRoute()->getIndexes();
	bool isAlias = false;

	if (!_request->getRoute()) {
		return std::vector<std::string>();
	}
	if (root.empty()) {
		root = _request->getServer()->getRoot();
	}
	if (!alias.empty()) {
		root = alias;
		isAlias = true;
	}
	if (path[path.size() - 1] != '/') {
		if (isAlias) {
			path = path.substr(_request->getRoute()->getPath().size());
		}
		allPaths.push_back(root + path);
	}
	for (size_t i = 0; i < indexes.size(); i++) {
		std::string index = indexes[i];
		std::string bkpPath = path;
		if (path == "/") {
			path = root + "/" + index;
		} else if (isAlias) {
			path = root + "/" + index;
		} else {
			path = root + path + index;
		}
		allPaths.push_back(path);
		path = bkpPath;
	}
	return allPaths;
}

std::vector<std::string> Response::getAllPathsInServer() {
	std::string path = _request->getPath();
	std::string root = _request->getServer()->getRoot();
	std::vector<std::string> indexes = _request->getServer()->getIndexes();
	std::vector<std::string> allPaths;

	if (path[path.size() - 1] != '/') {
		allPaths.push_back(root + path);
	}
	for (size_t i = 0; i < indexes.size(); i++)	{
		std::string index = indexes[i];
		std::string bkpPath = path;
		if (path == "/") {
			path = root + "/" + index;
		} else {
			path = root + path + index;
		}
		allPaths.push_back(path);
		path = bkpPath;
	}
	return allPaths;
}

bool Response::isLargeFile(std::string const &path) {
	struct stat fileStat;
	if (stat(path.c_str(), &fileStat) != 0) 	{
		Logger::log(Logger::ERROR, "Failed to stat file: %s", path.c_str());
		return false;
	}
	return fileStat.st_size > THRESHOLD_LARGE_FILE;
}

void Response::handleNotFound(std::string directoryToCheck) {
	std::string page;
	if (Utils::directoryExist(directoryToCheck.c_str())) {
		page = ErrorPage::getPage(403, _request->getServer()->getErrorPages());
	} else {
		page = ErrorPage::getPage(404, _request->getServer()->getErrorPages());
	}
	_response = page;
	setState(Response::FINISH);
}

std::string Response::findGoodPath(std::vector<std::string> allPaths) {
	for (size_t i = 0; i < allPaths.size(); i++) {
		Logger::log(Logger::DEBUG, "Trying to open file %s", allPaths[i].c_str());
		if (Utils::fileExists(allPaths[i])) {
			return allPaths[i];
		}
	}
	return "";
}

void Response::setHeaderChunked(std::string const &path) {
	_response = "HTTP/1.1 200 OK\r\n";
	_response += "Content-Type: " + Utils::getMimeType(path) + "\r\n";
	_response += "Transfer-Encoding: chunked\r\n";
	_response += "\r\n";
	_fileFd = open(path.c_str(), O_RDONLY);
	if (_fileFd == FAIL) {
		Logger::log(Logger::ERROR, "Failed to open file: %s", path.c_str());
		return handleNotFound(path);
	}
}

void Response::prepareChunkedResponse(std::string const &path) {
	if (_state != Response::CHUNK || _fileFd == FAIL){
		setHeaderChunked(path);
	}
	char buffer[RESPONSE_READ_BUFFER_SIZE];
	ssize_t bytesRead = read(_fileFd, buffer, RESPONSE_READ_BUFFER_SIZE);
	if (bytesRead == FAIL) {
		Logger::log(Logger::ERROR, "Failed to read file: %s", path.c_str());
		handleNotFound(path);
	} else if (bytesRead == 0) {
		close(_fileFd);
		_fileFd = FAIL;
		_response += "0\r\n\r\n";
		setState(Response::FINISH);
	} else {
		Logger::log(Logger::DEBUG, "[ChunkFUNC] Read %ld bytes from file %s", bytesRead, path.c_str());
		std::string chunkSize = Utils::intToHex(bytesRead) + "\r\n";
		std::string chunkData(buffer, bytesRead);
		_response += chunkSize + chunkData + "\r\n";
	}
}

void Response::prepareStandardResponse(std::string const &path) {
	Logger::log(Logger::DEBUG, "[prepareStandardResponse] Opening file %s", path.c_str());
	std::ifstream file(path.c_str());
	if (file.is_open()) {
		std::stringstream content;
		content << file.rdbuf();
		std::string body = content.str();
		_response = "HTTP/1.1 200 OK\r\n";
		_response += "Content-Type: " + Utils::getMimeType(path) + "\r\n";
		_response += "Content-Length: " + Utils::intToString(body.size()) + "\r\n";
		if (_request->_routeBlock) {
			_response += "Upload-Path: " + _request->_routeBlock->getUploadPath() + "\r\n";
		}
		_response += "\r\n";
		_response += body;
		file.close();
	} else {
		Logger::log(Logger::ERROR, "Failed to open file: %s", path.c_str());
		handleNotFound(path);
	}
}

void Response::handleRoute() {
	std::string root;
	_request->getRoute()->getRoot().empty() ? root = _request->getServer()->getRoot()
											   : root = _request->getRoute()->getRoot();
	std::vector<std::string> allPathsInRoute = getAllPathsInRoute();
	std::string path = findGoodPath(allPathsInRoute);

	if (path.empty()) {
		if (_request->getRoute()->getAutoIndex() == true) {
			std::string alias = _request->getRoute()->getAlias();
			if (!alias.empty()) {
				std::string shortPath = _request->getPath().substr(_request->getRoute()->getPath().size());
				_response = Utils::listDirectory(alias + shortPath, alias);
			} else {
				_response = Utils::listDirectory(root + _request->getPath(), root);
			}
			setState(Response::FINISH);
			return;
		}
		return handleNotFound(root + _request->getPath());
	}
	if (isLargeFile(path)){
		prepareChunkedResponse(path);
		setState(Response::CHUNK);
	} else {
		prepareStandardResponse(path);
		setState(Response::FINISH);
	}
}

void Response::handleServer() {
	std::vector<std::string> allPathsServer = getAllPathsInServer();
	std::string path = findGoodPath(allPathsServer);

	if (path.empty()) {
		return handleNotFound(_request->getServer()->getRoot() + _request->getPath());
	}
	if (isLargeFile(path)) {
		prepareChunkedResponse(path);
		setState(Response::CHUNK);
	} else {
		prepareStandardResponse(path);
		setState(Response::FINISH);
	}
}

void Response::handleGetRequest() {
	if (_request->getRoute()) {
		handleRoute();
	} else {
		handleServer();
	}
}

void Response::handlePostRequest() {
	std::string json = "{\n";
	if (_request->_routeBlock->getPath() == "/auth") {
		json += "\"message\": \"User logged in.\"\n";
	} else if (_request->_routeBlock->getPath() == "/auth/reset") {
		json += "\"message\": \"User logged out.\"\n";
	} else {
		json += "\"message\": \"File uploaded successfully.\",\n";
		json += "\"filepath\": \"" + _request->_body.getPath() + "\",\n";
		json += "\"size\": " + Utils::ullToStr(_request->_body.getSize()) + "\n";
	}
	json += "}\n";
	_response = "HTTP/1.1 200 OK\r\n";
	_response += "Content-Type: application/json\r\n";
	_response += "Content-Length: " + Utils::intToString(json.size()) + "\r\n";
	if (_request->_routeBlock->getPath() == "/auth") {
		_response += "Set-Cookie: access=granted; Path=/; Max-Age=300\r\n";
	}
	if (_request->_routeBlock->getPath() == "/auth/reset") {
		_response += "Set-Cookie: access=granted; Path=/; Max-Age=0\r\n";
	}
	_response += "\r\n";
	_response += json;
	setState(Response::FINISH);
}

void Response::handleDeleteRequest() {
	std::string path = _request->_routeBlock ? _request->_routeBlock->getRoot() + _request->getPath() : _request->getServer()->getRoot() + _request->getPath();
	if (!Utils::fileExists(path)) {
		return setError(404);
	}
	if (Utils::directoryExist(path.c_str()) || remove(path.c_str()) != 0) {
		return setError(403);
	}
	std::string json = "{\n";
	json += "\"message\": \"File deleted successfully.\",\n";
	json += "\"filename\": \"" + _request->getPath() + "\"\n";
	json += "}\n";
	_response = "HTTP/1.1 200 OK\r\n";
	_response += "Content-Type: application/json\r\n";
	_response += "Content-Length: " + Utils::intToString(json.size()) + "\r\n";
	_response += "\r\n";
	_response += json;
	setState(Response::FINISH);
}

void Response::handlePutRequest() {
	std::string json = "{\n";
	json += "\"message\": \"File uploaded successfully.\",\n";
	json += "\"filename\": \"" + _request->_body.getPath() + "\",\n";
	json += "\"size\": " + Utils::ullToStr(_request->_body.getSize()) + "\n";
	json += "}\n";
	_response = "HTTP/1.1 200 OK\r\n";
	_response += "Content-Type: application/json\r\n";
	_response += "Content-Length: " + Utils::intToString(json.size()) + "\r\n";
	_response += "\r\n";
	_response += json;
	setState(Response::FINISH);
}

int Response::generateResponse() {
	if (!_response.empty()) {
		_response.clear();
	}
	if (_request->getStateCode() != REQUEST_DEFAULT_STATE_CODE) {
		return setError(_request->getStateCode()), OK;
	}
	if (isRedirect()) {
		return setState(Response::FINISH), OK;
	}
	if (_request->isCgi()) {
		Logger::log(Logger::DEBUG, "IT'S A CGI");
		return handleCgi();
	}
	Logger::log(Logger::DEBUG, "IT'S NOT A CGI");
	if (_state != Response::CHUNK) {
		setState(Response::PROCESS);
	} 
	if (_request->getMethod() == "GET") {
		handleGetRequest();
	} else if (_request->getMethod() == "POST") {
		handlePostRequest();
	} else if (_request->getMethod() == "DELETE") {
		handleDeleteRequest();
	} else if (_request->getMethod() == "PUT") {
		handlePutRequest();
	} else {
		return setError(405), OK;
	}
	return OK;
}

void Response::setState(responseState state) {
	if (_state == Response::FINISH) {
		return Logger::log(Logger::DEBUG, "[Response::setState] Response already finished");
	}
	if (_state == state) {
		return Logger::log(Logger::DEBUG, "[Response::setState] Response already in this state");
	}
	_state = state;
	if (_state == Response::INIT) {
		Logger::log(Logger::DEBUG, "[Response::setState] Response INIT");
	} else if (_state == Response::PROCESS) {
		Logger::log(Logger::DEBUG, "[Response::setState] Response PROCESS");
	} else if (_state == Response::FINISH) {
		Logger::log(Logger::DEBUG, "[Response::setState] Response FINISH");
	}
}

void Response::setError(int code, bool generatePage) {
	_request->setStateCode(code);
	if (generatePage) {
		_response = ErrorPage::getPage(code, _request->getServer()->getErrorPages());
	}
	setState(Response::FINISH);
}

int Response::handleCgi() {
	Logger::log(Logger::DEBUG, "[Reponse::handleCgi] Handling CGI response");
	if (_state == Response::FINISH) {
		return FAIL;
	}
	bool isInit = _state == Response::INIT;
	if (_state == Response::INIT) {
		if (lseek(_request->_cgi._cgiHandler->getFdOut(), 0, SEEK_SET) == FAIL) {
			return setError(500), OK;
		}
		setState(Response::PROCESS);
	}
	char buffer[RESPONSE_READ_BUFFER_SIZE] = {0};
	memset(buffer, 0, RESPONSE_READ_BUFFER_SIZE);
	ssize_t bytesRead = read(_request->_cgi._cgiHandler->getFdOut(), buffer, RESPONSE_READ_BUFFER_SIZE - 1);
	if (bytesRead == FAIL) {
		if (isInit) {
			return setError(500), OK;
		}
		throw IntException(500);
	}
	if (bytesRead == 0)	{
		Logger::log(Logger::DEBUG, "[Reponse::handleCgi] No more data to read");
		if (_cgiHandler._isChunked)
			_response += "0\r\n\r\n";
		return setState(Response::FINISH), OK;
	}
	buffer[bytesRead] = '\0';
	std::string str(buffer, bytesRead);
	_cgiHandler.parse(str);
	if (_response.empty()) {
		return FAIL;
	}
	return OK;
}


int Response::getState() const { return _state; }

std::string Response::getResponse() const { return _response; }

size_t Response::getResponseSize() const { return _response.size(); }

CgiHandler &Response::getCgiHandler(void) { return _cgiHandler; }