#include "ConfigParser.hpp"

ConfigParser::ConfigParser() : _filename("") {} 

ConfigParser::~ConfigParser() {} 


std::vector<std::string> ConfigParser::supportedMethods = ConfigParser::getSupportedHttpMethods();

std::vector<std::string> ConfigParser::supportedHttpVersions = ConfigParser::getSupportedHttpVersions();

bool ConfigParser::isStartBlockConfigServer(std::vector<std::string> tokens) {
	return ((tokens.size() == 2 && tokens[0] == "server" && tokens[1] == "{") 
		|| (tokens.size() == 1 && tokens[0] == "server{"));
}

void ConfigParser::checkDoubleServerName() {
	std::vector<std::string> serverNames;
	for (size_t i = 0; i < _servers.size(); i++) {
		std::vector<std::string> currNames = _servers[i].getServerNames();
		for (size_t j = i + 1; j < _servers.size(); j++) {
			if (_servers[j].isServerNamePresent(currNames)) {
				Logger::log(Logger::FATAL, "conflicting server name \"%s\" on %s", currNames[0].c_str(), currNames[1].c_str());
			}
		}
	}
}

void ConfigParser::assignConfigs() {
	for (size_t i = 0; i < _servers.size(); i++) {
		std::map<std::string, ConfigListener> listens = _servers[i].getListeners();
		for (std::map<std::string, ConfigListener>::iterator it = listens.begin(); it != listens.end(); it++) {
			_configs[it->first].push_back(_servers[i]);
		}	
	}
}

void ConfigParser::parse(std::string const &filename) {
	_filename = filename;
	Logger::log(Logger::DEBUG, "Parsing config file: %s", _filename.c_str());
	std::ifstream configFile(_filename.c_str());
	std::vector<std::string> tokens;
	std::string line;

	if (!configFile.is_open()) {
		Logger::log(Logger::FATAL, "File %s can't be opened or doesn't exist", _filename.c_str());
	}
	while (std::getline(configFile, line)) {
		ConfigParser::fileLineCount++;
		line = Utils::trimLine(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}
		tokens = Utils::split(line, " ");
		if (isStartBlockConfigServer(tokens)) {
			BlockConfigServer server(_filename);
			_servers.push_back(server.getServerConfig(configFile));
		} else {
			Logger::log(Logger::FATAL, "Invalid line: \"%s\" in file: %s:%d", line.c_str(), _filename.c_str(), ConfigParser::fileLineCount);
		}
	}
	if (_servers.size() == 0){
		BlockConfigServer server(_filename);
		_servers.push_back(server.getServerConfig(configFile));
	}
	checkDoubleServerName();
	assignConfigs();
	configFile.close();
}

void ConfigParser::printServers() { 	
	for (size_t i = 0; i < _servers.size(); i++) {
		std::cout << "************ SERVER " << i + 1 << " ************\n"
				  << std::endl;
		_servers[i].printServer();
		std::cout << std::endl;
	}
}

std::vector<std::string> ConfigParser::getSupportedHttpMethods() {
	std::vector<std::string> methods;
	methods.push_back("GET");
	methods.push_back("POST");
	methods.push_back("DELETE");
	methods.push_back("PUT");
	return methods;
}

bool ConfigParser::isMethodSupported(std::string method) {
	return std::find(ConfigParser::supportedMethods.begin(), ConfigParser::supportedMethods.end(), method) != ConfigParser::supportedMethods.end();
}

std::vector<std::string> ConfigParser::getSupportedHttpVersions() {
	std::vector<std::string> versions;
	versions.push_back("HTTP/1.0");
	versions.push_back("HTTP/1.1");
	return versions;
}

bool ConfigParser::isHttpVersionSupported(std::string version) {
	return std::find(ConfigParser::supportedHttpVersions.begin(), ConfigParser::supportedHttpVersions.end(), version) != ConfigParser::supportedHttpVersions.end();
}

std::map<std::string, std::vector<BlockConfigServer> > &ConfigParser::getServers() { return _configs; }
