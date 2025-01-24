#include "BlockConfigServer.hpp"

BlockConfigServer::BlockConfigServer() : _clientMaxBodySize(BODY_SIZE_MAX), _filename("") {
	_counters["root"] = 0;
	_counters["clientMaxBodySize"] = 0;
}

BlockConfigServer::BlockConfigServer(std::string filename) : _clientMaxBodySize(BODY_SIZE_MAX), _filename(filename) {
	_counters["root"] = 0;
	_counters["clientMaxBodySize"] = 0;
}

BlockConfigServer::BlockConfigServer(BlockConfigServer const &obj) { *this = obj; }

BlockConfigServer::~BlockConfigServer(){}

BlockConfigServer &BlockConfigServer::operator=(BlockConfigServer const &obj) {
	if (this != &obj) {
		_listeners = obj._listeners;
		_serverNames = obj._serverNames;
		_indexes = obj._indexes;
		_root = obj._root;
		_clientMaxBodySize = obj._clientMaxBodySize;
		_routes = obj._routes;
		_errorPages = obj._errorPages;
		_filename = obj._filename;
		_counters = obj._counters;
	}
	return *this;
}


void BlockConfigServer::setClientMaxBodySize(std::string clientMaxBodySize) {
	_clientMaxBodySize = Utils::strToUll(clientMaxBodySize);
	_counters["clientMaxBodySize"]++;
}


bool BlockConfigServer::isStartBlockConfigRoute(std::vector<std::string>& tokens) {
	return (tokens.size() == 3 && tokens[0] == "route" && tokens[2] == "{");
}

void BlockConfigServer::addListener(std::string &token) {
	ConfigListener listen(token);
	
	if (_listeners.find(listen.getAddressPortCombined()) != _listeners.end()) {
		Logger::log(Logger::FATAL, "Duplicate listen in server context: %s", token.c_str());
	}
	_listeners[listen.getAddressPortCombined()] = listen;
}

void BlockConfigServer::addServerName(std::vector<std::string>& token) {
	for (size_t i = 1; i < token.size(); i++) {
		if (std::find(_serverNames.begin(), _serverNames.end(), token[i]) == _serverNames.end()) {
			_serverNames.push_back(token[i]);
		}
	}
}

void BlockConfigServer::addIndexes(std::vector<std::string>& token) {
	for (size_t i = 1; i < token.size(); i++) {
		if (std::find(_indexes.begin(), _indexes.end(), token[i]) == _indexes.end()) {
			_indexes.push_back(token[i]);
		}
	}
}

void BlockConfigServer::addErrorPages(int errorCode, std::string file) { 
	if (errorCode < 400 || errorCode > 599) {
		Logger::log(Logger::FATAL, "Invalid error code: %d in file %s:%d", errorCode, _filename.c_str(), ConfigParser::fileLineCount);
	}
	_errorPages[errorCode] = file; 
}

bool BlockConfigServer::isServerNamePresent(std::vector<std::string> &names) {
	for (size_t i = 0; i < names.size(); i++) {
		for (size_t j = 0; j < _serverNames.size(); j++) {
			if (names[i] == _serverNames[j]) {
				names.clear();
				names.push_back(_serverNames[j]);
				names.push_back(_listeners.begin()->first);
				return true;
			}
		}
	}
	return false;
}

void BlockConfigServer::cleanPaths() {
	if (!_root.empty() && _root != "/" && _root[_root.size() - 1] == '/') {
		_root.erase(_root.size() - 1);
	}
	for (std::map<int, std::string>::iterator it = _errorPages.begin(); it != _errorPages.end(); it++) {
		if (it->second != "/" && it->second[it->second.size() - 1] == '/') {
			it->second.erase(it->second.size() - 1);
		}
	}
	std::vector<BlockConfigRoute>::iterator it;
	for (it = _routes.begin(); it != _routes.end(); it++) {
		it->cleanPaths();
	}
}

void BlockConfigServer::checkDoubleLine() {
	std::map<std::string, int>::iterator it;
	for (it = _counters.begin(); it != _counters.end(); it++) {
		if (it->second > 1) {
			Logger::log(Logger::FATAL, "Duplicate line in server context: %s", it->first.c_str());
		}
	}
}

void BlockConfigServer::checkDoubleRoute() {
	std::vector<BlockConfigRoute>::iterator it;
	std::vector<BlockConfigRoute>::iterator it2;
	for (it = _routes.begin(); it != _routes.end(); ++it) {
		for (it2 = it + 1; it2 != _routes.end(); ++it2) {
			if (it->getPath() == it2->getPath()) {
				Logger::log(Logger::FATAL, "Duplicate route: \"%s\" in file: %s", it->getPath().c_str(), _filename.c_str());
			}
		}
	}
}

void BlockConfigServer::setDefaultValue() {
	if (_listeners.empty()) {
		ConfigListener listener("0.0.0.0:1234");
		_listeners["0.0.0.0:1234"] = listener;
	}
	if (_root.empty()) {
		_root = "./w3/pretty";
	}
	if (_indexes.empty()) {
		_indexes.push_back("index.html");
	}
}

bool BlockConfigServer::isValidLineServer(std::vector<std::string>& tokens, std::string& key, std::ifstream &configFile) {
	if (tokens.size() < 2) {
		return false;
	}
	if (isStartBlockConfigRoute(tokens)) {
		BlockConfigRoute route(_filename);
		addRoute(route.getRouteConfig(configFile, tokens[1]));
	} else if (key == "listen" && tokens.size() == 2) {
		addListener(tokens[1]);
	} else if (key == "serverName") {
		addServerName(tokens);
	} else if (key == "index") {
		addIndexes(tokens);
	} else if (key == "root" && tokens.size() == 2) {
		setRoot(tokens[1]);
	} else if (key == "bodySizeMax" && tokens.size() == 2) {
		setClientMaxBodySize(tokens[1]);
	} else if (key == "errorPage" && tokens.size() == 3) {
		addErrorPages(std::atoi(tokens[1].c_str()), tokens[2]);
	} else {
		return false;
	}
	return true;
}

BlockConfigServer BlockConfigServer::getServerConfig(std::ifstream &configFile) {
	std::string line;
	std::vector<std::string> tokens;
	std::string key;
	bool isCloseServer = false;

	while (std::getline(configFile, line)) {
		ConfigParser::fileLineCount++;
		line = Utils::trimLine(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}
		tokens = Utils::split(line, " ");
		key = tokens[0];
		if (key[0] == '}' && key.size() == 1 && tokens.size() == 1) {
			isCloseServer = true;
			break;
		} else if (isValidLineServer(tokens, key, configFile)) {
			continue;
		} else {
			Logger::log(Logger::FATAL, "Invalid line: \"%s\" in file: %s:%d", line.c_str(), _filename.c_str(), ConfigParser::fileLineCount);
		}
	}
	if (isCloseServer == false && !Utils::isEmptyFile()) {
		Logger::log(Logger::FATAL, "Missing } in file %s:%d", _filename.c_str(), ConfigParser::fileLineCount);
	}
	checkDoubleLine();
	setDefaultValue();
	checkDoubleRoute();
	cleanPaths();
	return *this;
}

BlockConfigRoute *BlockConfigServer::findRoute(std::string const &uri) {
	for (std::vector<BlockConfigRoute>::iterator it = _routes.begin(); it != _routes.end(); it++) {
		if (uri.find(it->getPath()) == 0) {
			return &(*it);
		}
	}
	return NULL;
}

void BlockConfigServer::printPair(std::string const &label, std::string const &value) {
    std::cout << std::left << label << ": " << (value.empty() ? "none" : value) << std::endl;
}

void BlockConfigServer::printInt(std::string const &label, int value) {
    std::cout << std::left << label << ": " << (value == -1 ? "none" : Utils::intToString(value)) << std::endl;
}

void BlockConfigServer::printVector(std::string const &label, const std::vector<std::string>& vec) {
    std::cout << std::left << label << ": " << (vec.empty() ? "none" : "") << std::endl;
    for (std::vector<std::string>::const_iterator it = vec.begin(); it != vec.end(); it++) {
        std::cout << "\t- " << *it << std::endl;
	}
}

void BlockConfigServer::printMap(std::string const &label, const std::map<int, std::string> &map) {
    std::cout << std::left << label << ": " << (map.empty() ? "none" : "") << std::endl;
    for (std::map<int, std::string>::const_iterator it = map.begin(); it != map.end(); it++) {
        std::cout << "\t- " << it->first << ": " << it->second << std::endl;
	}
}

void BlockConfigServer::printListeners() {
    std::cout << std::left << "Listens" << ": " << (_listeners.empty() ? "none" : "") << std::endl;
    for (std::map<std::string, ConfigListener>::const_iterator it = _listeners.begin(); it != _listeners.end(); it++) {
        std::cout << "\t- " << it->second.getAddress() << ":" << it->second.getPort() << std::endl;
	}
}

void BlockConfigServer::printServer() {
    printVector("Server names", _serverNames);
    printListeners();
    printVector("Indexes", _indexes);
    printPair("Root", _root);
	std::cout << "Client max body size: " << Utils::ullToStr(_clientMaxBodySize) << std::endl;
    printMap("Error pages", _errorPages);

    if (_routes.empty()) {
        std::cout << std::setw(25) << std::left << "Routes" << ": none" << std::endl;
    } else {
        int i = 0;
        for (std::vector<BlockConfigRoute>::iterator it = _routes.begin(); it != _routes.end(); it++) {
            std::cout << "\n-- ROUTE " << ++i << " --" << std::endl;
            it->printRoute();
        }
    }
}


void BlockConfigServer::incrementCounter(std::string const &key) { _counters[key]++; }

const std::map<int, std::string> &BlockConfigServer::getErrorPages() const { return _errorPages; }

const std::vector<std::string> &BlockConfigServer::getServerNames() const { return _serverNames; }

std::vector<BlockConfigRoute> *BlockConfigServer::getRoutes() { return &_routes; }

std::string const &BlockConfigServer::getRoot() const { return _root; }

unsigned long long BlockConfigServer::getClientMaxBodySize() const { return _clientMaxBodySize; }

const std::map<std::string, ConfigListener> &BlockConfigServer::getListeners() const { return _listeners; }

const std::vector<std::string> &BlockConfigServer::getIndexes() const { return _indexes; }

void BlockConfigServer::setRoot(std::string const &root) {
	_root = root;
	_counters["root"]++;
}

void BlockConfigServer::setRoutes(std::vector<BlockConfigRoute> const &routes) { _routes = routes; }

void BlockConfigServer::setErrorPages(std::map<int, std::string> const &errorPage) { _errorPages = errorPage; }

void BlockConfigServer::addRoute(BlockConfigRoute const &routes) { _routes.push_back(routes); }
