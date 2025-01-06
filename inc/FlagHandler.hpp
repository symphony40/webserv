#ifndef FLAGHANDLER_HPP
#define FLAGHANDLER_HPP

#include <iostream>
#include <map>

#include "Utils.hpp"

#define DEFAULT_CONFIG_FILE_PATH "./config/good/default.conf"

class FlagHandler {
private:
	int _argc;
	char **_argv;
	std::string _configFilePath;
	std::map<std::string, bool> _options;
	// int _state;

	std::map<std::string, bool> _generateOptions();
	std::string _convertToLongOption(std::string option);
	std::string _convertToShortOption(std::string option);
	void _parse();

public:
	FlagHandler(int argc, char **argv);
	~FlagHandler();

	bool isOption(std::string option);
	// int getState() const;
	std::string getConfigFilePath() const;
	void help();
	void summary();
};

#endif
