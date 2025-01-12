#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "BlockConfigServer.hpp"
#include "Utils.hpp"

class BlockConfigServer;

class ConfigParser {
public:
	static int fileLineCount;
	static std::vector<std::string> supportedMethods;
	static std::vector<std::string> supportedHttpVersions;
	
	ConfigParser();
	~ConfigParser();
	std::map<std::string, std::vector<BlockConfigServer> > &getServers();
	// std::map<std::string, std::vector<BlockConfigServer> > getConfigs() const;
	
	void parse(std::string const &filename);
	void checkDoubleServerName();
	bool isStartBlockConfigServer(std::vector<std::string> tokens);
	void assignConfigs();
	void printServers();
	static bool isMethodSupported(std::string method);
	static bool isHttpVersionSupported(std::string version);

private:
	std::string _filename;
	std::vector<BlockConfigServer> _servers;
	std::map<std::string, std::vector<BlockConfigServer> > _configs;

	static std::vector<std::string>	getSupportedHttpMethods();
	static std::vector<std::string>	getSupportedHttpVersions();
};

#endif
