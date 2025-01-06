#ifndef SERVER_H
#define SERVER_H

# include <iostream>
# include <map>
# include <vector>
# include <iomanip>

# include "Logger.hpp"
# include "Utils.hpp"
# include "ConfigParser.hpp"
# include "BlocLocation.hpp"
# include "ListenConfig.hpp"

# define CLIENT_MAX_BODY_SIZE 1048576 // 1MB

class BlocLocation;
class ListenConfig;

class BlocServer
{
private:
	// config bloc server
	std::map<std::string, ListenConfig> _listens;
	std::vector<std::string> _serverNames;
	std::vector<std::string> _indexes;
	std::string _root;
	unsigned long long _clientMaxBodySize;
	std::vector<BlocLocation> _locations;
	std::map<int, std::string> _errorPages;

	// divers
	std::string _filename;
	std::map<std::string, int> _counterView;

	// Methods
	void checkDoubleLine();
	void incrementCounter(const std::string &key) { _counterView[key]++; }
	bool isValidLineServer(std::vector<std::string>& tokens, std::string& key, std::ifstream &configFile);
	bool isStartBlocLocation(std::vector<std::string>& tokens);
	void checkDoubleLocation();
	void cleanPaths();

public:
	BlocServer(std::string filename);
	BlocServer(void);
	BlocServer(const BlocServer &other);
	~BlocServer(void);

	BlocServer &operator=(const BlocServer &other);


	// parsing
	BlocServer getServerConfig(std::ifstream &file_config);

	// Getters
	const std::map<int, std::string> &getErrorPages() const { return _errorPages; }
	const std::vector<std::string> &getServerNames() const { return _serverNames; }
	// const std::vector<BlocLocation> &getLocations() const { return _locations; }
	std::vector<BlocLocation>* getLocations() { return &_locations; }
	const std::string &getRoot() const { return _root; }
	unsigned long long getClientMaxBodySize() const { return _clientMaxBodySize; }
	const std::map<std::string, ListenConfig> &getListens() const { return _listens; }
	const std::vector<std::string> &getIndexes() const { return _indexes; }

	// Util
	bool isServerNamePresent(std::vector<std::string>& otherNames);


	// Setters
	void setClientMaxBodySize(std::string clientMaxBodySize);
	void setRoot(const std::string &root)
	{
		_root = root;
		_counterView["root"]++;
	}
	void setDefaultValue();
	void setLocations(const std::vector<BlocLocation> &locations) { _locations = locations; }
	void setErrorPages(const std::map<int, std::string> &errorPage) { _errorPages = errorPage; }

	// Adders
	void addErrorPages(int errorCode, std::string file);
	void addLocation(const BlocLocation &locations) { _locations.push_back(locations); }
	void addListen(std::string &token);
	void addServerName(std::vector<std::string>& token);
	void addIndexes(std::vector<std::string>& token);

	// Finders
	BlocLocation*	findLocation(const std::string &uri);

	// Print
	void printServer(void);
	void printListens();
	void printPair(const std::string& label, const std::string& value);
	void printInt(const std::string& label, int value);
	void printVector(const std::string& label, const std::vector<std::string>& vec);
	void printMap(const std::string& label, const std::map<int, std::string>& map);


};

#endif