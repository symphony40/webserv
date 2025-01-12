#ifndef SERVER_H
#define SERVER_H

#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

#include "BlockConfigLocation.hpp"
#include "ConfigParser.hpp"
#include "ConfigListener.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

#define CLIENT_MAX_BODY_SIZE 1048576 // 1MB

class BlockConfigLocation;
class ConfigListener;

class BlockConfigServer {
private:
	std::map<std::string, ConfigListener> _listeners;
	std::vector<std::string> _serverNames;
	std::vector<std::string> _indexes;
	std::string _root;
	unsigned long long _clientMaxBodySize;
	std::vector<BlockConfigLocation> _locations;
	std::map<int, std::string> _errorPages;
	std::string _filename;
	std::map<std::string, int> _counters;

	bool isStartBlockConfigLocation(std::vector<std::string> &tokens);
	bool isValidLineServer(std::vector<std::string> &tokens, std::string &key, std::ifstream &configFile);
	void checkDoubleLine();
	void checkDoubleLocation();
	void cleanPaths();
	void incrementCounter(std::string const &key);

public:
	BlockConfigServer(std::string filename);
	BlockConfigServer();
	BlockConfigServer(BlockConfigServer const &obj);
	~BlockConfigServer();
	BlockConfigServer &operator=(BlockConfigServer const &obj);

	BlockConfigLocation *findLocation(std::string const &uri);
	BlockConfigServer getServerConfig(std::ifstream &file_config);
	bool isServerNamePresent(std::vector<std::string> &names);
	const std::map<int, std::string> &getErrorPages() const;
	const std::map<std::string, ConfigListener> &getListeners() const;
	const std::vector<std::string> &getIndexes() const;
	const std::vector<std::string> &getServerNames() const;
	std::string const &getRoot() const;
	std::vector<BlockConfigLocation> *getLocations();
	unsigned long long getClientMaxBodySize() const;
	void addErrorPages(int errorCode, std::string file);
	void addIndexes(std::vector<std::string> &token);
	void addListener(std::string &token);
	void addLocation(const BlockConfigLocation &locations);
	void addServerName(std::vector<std::string>& token);
	void printInt(std::string const &label, int value);
	void printListeners();
	void printMap(std::string const &label, const std::map<int, std::string> &map);
	void printPair(std::string const &label, std::string const &value);
	void printServer();
	void printVector(std::string const &label, const std::vector<std::string> &vec);
	void setClientMaxBodySize(std::string clientMaxBodySize);
	void setDefaultValue();
	void setErrorPages(const std::map<int, std::string> &errorPage);
	void setLocations(const std::vector<BlockConfigLocation> &locations);
	void setRoot(std::string const &root);

};

#endif
