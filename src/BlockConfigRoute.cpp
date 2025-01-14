#include "BlockConfigRoute.hpp"

BlockConfigRoute::BlockConfigRoute(std::string filename) : _autoindex(false), _filename(filename) {
	_counters["root"] = 0;
	_counters["alias"] = 0;
	_counters["allowedMethods"] = 0;
	_counters["autoindex"] = 0;
	_counters["uploadPath"] = 0;
}

BlockConfigRoute::BlockConfigRoute(BlockConfigRoute const &obj) { *this = obj; }

BlockConfigRoute::~BlockConfigRoute() {}

BlockConfigRoute &BlockConfigRoute::operator=(BlockConfigRoute const &obj) {
	if (this != &obj) {
		_path = obj._path;
		_root = obj._root;
		_rewrite = obj._rewrite;
		_alias = obj._alias;
		_indexes = obj._indexes;
		_allowedHttpMethods = obj._allowedHttpMethods;
		_autoindex = obj._autoindex;
		_cgiExtension = obj._cgiExtension;
		_uploadPath = obj._uploadPath;
		_counters = obj._counters;
		_filename = obj._filename;
	}
	return *this;
}

void BlockConfigRoute::addAllowedMethods(std::vector<std::string> &tokens) {
	httpMethods method;
	incrementCounter("allowedMethods");
	for (size_t i = 1; i < tokens.size(); i++){
		std::string token = tokens[i];
		if (ConfigParser::isMethodSupported(token) == false) {
			Logger::log(Logger::FATAL, "Invalid method: \"%s\" in file: %s:%d", token.c_str(), _filename.c_str(), ConfigParser::fileLineCount);
		}
		if (token == "GET") {
			method = GET;
		} else if (token == "POST") {
			method = POST;
		} else if (token == "DELETE") {
			method = DELETE;
		} else if (token == "PUT") {
			method = PUT;
		} else {
			Logger::log(Logger::FATAL, "Invalid method: \"%s\" in file: %s:%d", token.c_str(), _filename.c_str(), ConfigParser::fileLineCount);
		}
		if (std::find(_allowedHttpMethods.begin(), _allowedHttpMethods.end(), method) != _allowedHttpMethods.end()) {
			Logger::log(Logger::FATAL, "Duplicate method: \"%s\" in file: %s:%d", token.c_str(), _filename.c_str(), ConfigParser::fileLineCount);
		}
		_allowedHttpMethods.push_back(method);
	}
}

bool BlockConfigRoute::strToBool(std::string &str) {
	if (str == "on") {
		return true;
	} else if (str == "off") {
		return false;
	} else {
		Logger::log(Logger::FATAL, "Invalid value for autoindex: \"%s\" in file: %s:%d", str.c_str(), _filename.c_str(), ConfigParser::fileLineCount);
	}
	return false;
}

void BlockConfigRoute::addIndexes(std::vector<std::string> &token) {
	for (size_t i = 1; i < token.size(); i++) {
		if (std::find(_indexes.begin(), _indexes.end(), token[i]) == _indexes.end()) {
			_indexes.push_back(token[i]);
		}
	}
}

void BlockConfigRoute::addCgiExtension(std::vector<std::string> &token) {
	if (_cgiExtension.find(token[1]) != _cgiExtension.end()) {
		Logger::log(Logger::FATAL, "Duplicate cgi extension: \"%s\" in file: %s:%d", token[1].c_str(), _filename.c_str(), ConfigParser::fileLineCount);
	}
	_cgiExtension[token[1]] = token[2];
}

void BlockConfigRoute::setRewrite(std::vector<std::string> &tokens) {
	int code = std::atoi(tokens[1].c_str());
	if (code < 300 || code > 399) {
		Logger::log(Logger::FATAL, "Invalid return code: \"%s\" in file: %s:%d", tokens[1].c_str(), _filename.c_str(), ConfigParser::fileLineCount);
	}
	_rewrite = std::make_pair(code, tokens[2]);
}

void BlockConfigRoute::cleanPaths() {
	if (_path != "/" && _path[_path.size() - 1] == '/') {
		_path.erase(_path.size() - 1);
	}
	if (!_root.empty() && _root != "/" && _root[_root.size() - 1] == '/') {
		_root.erase(_root.size() - 1);
	}
	if (!_alias.empty() && _alias != "/" && _alias[_alias.size() - 1] == '/') {
		_alias.erase(_alias.size() - 1);	
	}
	if (!_uploadPath.empty() && _uploadPath != "/" && _uploadPath[_uploadPath.size() - 1] == '/') {
		_uploadPath.erase(_uploadPath.size() - 1);
	}
	for (std::map<std::string, std::string>::iterator it = _cgiExtension.begin(); it != _cgiExtension.end(); it++) {
		if (it->second != "/" && it->second[it->second.size() - 1] == '/') {
			it->second.erase(it->second.size() - 1);
		}
	}
}

void BlockConfigRoute::checkDoubleLine() {
	std::map<std::string, int>::iterator it;
	for (it = _counters.begin(); it != _counters.end(); it++) {
		if (it->second > 1) {
			Logger::log(Logger::FATAL, "Duplicate line in route context: %s", it->first.c_str());
		}
	}
	if (_counters["root"] > 0 && _counters["alias"] > 0) {
		Logger::log(Logger::FATAL, "Alias and Root can't be set in same route bloc %s", _path.c_str());
	}
}

void BlockConfigRoute::setDefaultValues() {
	if (_allowedHttpMethods.size() == 0) {
		_allowedHttpMethods.push_back(GET);
		_allowedHttpMethods.push_back(POST);
		_allowedHttpMethods.push_back(DELETE);
	}
	if (_indexes.size() == 0) {
		_indexes.push_back("index.html");
	}
}

bool BlockConfigRoute::isValidLineLocation(std::vector<std::string> &tokens, std::string &key) {
	if (tokens.size() < 2) {
		return false;
	}
	if (key == "root" && tokens.size() == 2) {
		setRoot(tokens[1]);
	} else if (key == "autoindex") {
		setAutoIndex(strToBool(tokens[1]));
	} else if (key == "return" && tokens.size() == 3) {
		setRewrite(tokens);
	} else if (key == "alias" && tokens.size() == 2) {
		setAlias(tokens[1]);
	} else if (key == "allow") {
		addAllowedMethods(tokens);
	} else if (key == "index") {
		addIndexes(tokens);
	} else if (key == "cgi" && tokens.size() == 3) {
		addCgiExtension(tokens);
	} else if (key == "uploadPath" && tokens.size() == 2) {
		setUploadPath(tokens[1]);
	} else {
		return false;
	}
	return true;
}

BlockConfigRoute BlockConfigRoute::getLocationConfig(std::ifstream &configFile, std::string &path) {
	std::string line;
	std::vector<std::string> tokens;
	std::string key;
	bool isCloseLocation = false;

	setPath(path);
	while (std::getline(configFile, line)) {
		ConfigParser::fileLineCount++;
		line = Utils::trimLine(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}
		tokens = Utils::split(line, " ");
		key = tokens[0];
		if (key[0] == '}' && key.size() == 1 && tokens.size() == 1) {
			isCloseLocation = true;
			break;
		}
		if (isValidLineLocation(tokens, key)) {
			continue;
		} else {
			Logger::log(Logger::FATAL, "Invalid line: \"%s\" in file: %s:%d", line.c_str(), _filename.c_str(), ConfigParser::fileLineCount);
		}
	}
	if (!isCloseLocation && !Utils::isEmptyFile()) {
		Logger::log(Logger::FATAL, "Missing } in file: %s:%d", _filename.c_str(), ConfigParser::fileLineCount);
	}
	checkDoubleLine();
	setDefaultValues();
	return *this;
}

void BlockConfigRoute::printPair(std::string const &label, std::string const &value) {
	std::cout << std::setw(15) << std::left << label << ": " << (value.empty() ? "none" : value) << std::endl;
}

void BlockConfigRoute::printBool(std::string const &label, bool value, std::string const &trueString, std::string const &falseString) {
	std::cout << std::setw(15) << std::left << label << ": " << (value ? trueString : falseString) << std::endl;
}

void BlockConfigRoute::printVector(std::string const &label, std::vector<std::string> const &vector) {
	std::cout << std::setw(15) << std::left << label << ": " << (vector.empty() ? "none" : "") << std::endl;
	for (std::vector<std::string>::const_iterator it = vector.begin(); it != vector.end(); it++) {
		std::cout << "\t- " << *it << std::endl;
	}
}

void BlockConfigRoute::printMap(std::string const &label, const std::map<std::string, std::string> &map) {
	std::cout << std::setw(15) << std::left << label << ": " << (map.empty() ? "none" : "") << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = map.begin(); it != map.end(); it++) {
		std::cout << "\t- " << it->first << ": " << it->second << std::endl;
	}
}


void BlockConfigRoute::printLocation() {
	printPair("Path", _path);
	printPair("Root", _root);
	printPair("Alias", _alias);
	printMap("CGI extension", _cgiExtension);
	printPair("Upload path", _uploadPath);
	printBool("Autoindex", _autoindex == true, "on", "off");

	std::cout << std::setw(15) << std::left << "Rewrite" << ": " 
			  << (_rewrite.first != 0 ? Utils::intToString(_rewrite.first) + " " + _rewrite.second : "none") 
			  << std::endl;

	printVector("Indexes", _indexes);

	std::cout << std::setw(15) << std::left << "Allowed methods" << ": " << std::endl;
	for (std::vector<httpMethods>::const_iterator it = _allowedHttpMethods.begin(); it != _allowedHttpMethods.end(); it++) {
		std::cout << "\t- ";
		if (*it == GET) {
			std::cout << "GET";
		} else if (*it == POST) {
			std::cout << "POST";
		} else if (*it == DELETE) {
			std::cout << "DELETE";
		} else if (*it == PUT) {
			std::cout << "PUT";
		} else {
			std::cout << "UNKNOWN";
		}
		std::cout << std::endl;
	}
}

bool BlockConfigRoute::isMethodAllowed(httpMethods method) {
	return std::find(_allowedHttpMethods.begin(), _allowedHttpMethods.end(), method) != _allowedHttpMethods.end();
}

httpMethods	BlockConfigRoute::converStrToMethod(std::string const &method) {
	if (method == "GET") {
		return GET;
	} else if (method == "POST") {
		return POST;
	} else if (method == "DELETE") {
		return DELETE;
	} else if (method == "PUT") {
		return PUT;
	} else {
		return UNKNOWN;
	}
}


bool BlockConfigRoute::getAutoIndex() const { return _autoindex; }

bool BlockConfigRoute::isCgi(std::string const &path) const { return _cgiExtension.find(path) != _cgiExtension.end(); }

const std::map<std::string, std::string> &BlockConfigRoute::getCGI() const { return _cgiExtension; }

const std::pair<int, std::string> &BlockConfigRoute::getRewrite() const { return _rewrite; }

const std::vector<httpMethods> &BlockConfigRoute::getAllowedMethods() const { return _allowedHttpMethods; }

const std::vector<std::string> &BlockConfigRoute::getFiles() const { return _indexes; }

const std::vector<std::string> &BlockConfigRoute::getIndexes() const { return _indexes; }

std::string	BlockConfigRoute::getCgiPath(std::string const &path) const { return _cgiExtension.at(path); }

std::string const &BlockConfigRoute::getAlias() const { return _alias; }

std::string const &BlockConfigRoute::getPath() const { return _path; }

std::string const &BlockConfigRoute::getRoot() const { return _root; }

void BlockConfigRoute::setAlias(std::string const &alias) { _alias = alias;  _counters["alias"]++;}

void BlockConfigRoute::setAutoIndex(bool autoindex) { _autoindex = autoindex;  _counters["autoindex"]++;}

void BlockConfigRoute::setPath(std::string const &path) { _path = path; }

void BlockConfigRoute::setRoot(std::string const &root) { _root = root;  _counters["root"]++;}

void BlockConfigRoute::setUploadPath(std::string const &uploadPath) { _uploadPath = uploadPath; _counters["uploadPath"]++;}
