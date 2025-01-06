#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "Utils.hpp"
#include "BlocServer.hpp"

class BlocServer;

class ConfigParser {
public:
	static int countFileLines;
	static std::vector<std::string> supportedMethods;
	static std::vector<std::string> supportedHttpVersions;
	
	ConfigParser();
	~ConfigParser();
	std::map<std::string, std::vector<BlocServer> > &getServers();
	// std::map<std::string, std::vector<BlocServer> > getConfigs() const;
	
	void parse(const std::string &filename);
	void checkDoubleServerName();
	bool isStartBlocServer(std::vector<std::string> tokens);
	void assignConfigs();
	void printServers();
	static bool isMethodSupported(std::string method);
	static bool isHttpVersionSupported(std::string version);

private:
	std::string _filename;
	std::vector<BlocServer> _servers;
	std::map<std::string, std::vector<BlocServer> > _configs;

	static std::vector<std::string>	_getSupportedMethods();
	static std::vector<std::string>	_getSupportedHttpVersions();
};

#endif
