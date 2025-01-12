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

	std::map<std::string, bool> generateOptions();
	std::string convertToLongOption(std::string option);
	std::string convertToShortOption(std::string option);
	void parse();

public:
	FlagHandler(int argc, char **argv);
	~FlagHandler();

	bool isOption(std::string option);
	std::string getConfigFilePath() const;
	void help();
	void summary();
};

#endif
