#include "FlagHandler.hpp"

FlagHandler::FlagHandler(int argc, char **argv) : _argc(argc), _argv(argv), _configFilePath(DEFAULT_CONFIG_FILE_PATH), _options(generateOptions()) {
	parse();
	if (isOption("--quiet")) {
		Logger::setLogState(false);
	}
	if (isOption("--log")) {
		Logger::setLogFileState(true);
	}
	if (isOption("--debug")) {
		Logger::setLogDebugState(true);
	}
	if (isOption("--summary")) {
		summary();
	}
}

FlagHandler::~FlagHandler() {}

std::map<std::string, bool> FlagHandler::generateOptions() {
	std::map<std::string, bool> options;
	options["--debug"] = false;
	options["--help"] = false;
	options["--log"] = false;
	options["--quiet"] = false;
	options["--summary"] = false;
	return (options);
}

std::string FlagHandler::convertToLongOption(std::string option) {
	if (option == "-d")	return ("--debug");
	if (option == "-h")	return ("--help");
	if (option == "-l")	return ("--log");
	if (option == "-q")	return ("--quiet");
	if (option == "-s")	return ("--summary");
	return (option);
}

std::string FlagHandler::convertToShortOption(std::string option) {
	if (option == "--debug") return ("-d");
	if (option == "--help") return ("-h");
	if (option == "--log") return ("-l");
	if (option == "--quiet") return ("-q");
	if (option == "--summary") return ("-s");
	return (option);
}

void FlagHandler::parse() {
	try {
		for (int i = 1; i < _argc; i++) {
			std::string arg = _argv[i];

			if ((arg.size() == 2 && arg[0] == '-') || (arg.size() > 2 && arg.substr(0, 2) == "--")) {
				if (arg == "-h" || arg == "--help")
					_options["--help"] = true;
				else if (arg == "-q" || arg == "--quiet")
					_options["--quiet"] = true;
				else if (arg == "-l" || arg == "--log")
					_options["--log"] = true;
				else if (arg == "-d" || arg == "--debug")
					_options["--debug"] = true;
				else if (arg == "-s" || arg == "--summary")
					_options["--summary"] = true;
				else
					Logger::log(Logger::DEBUG, "illegal option -- %s", arg.substr(2).c_str());
			} else {
				if (_configFilePath == DEFAULT_CONFIG_FILE_PATH) {
					_configFilePath = arg;
				} else {
					Logger::log(Logger::DEBUG, "invalid argument -- %s (config file already set: \"%s\")", arg.c_str(), _configFilePath.c_str());
				}
			}
		}
	} catch (const std::exception &e) {
		Utils::printMessage(std::cerr, "%s : %s\n", _argv[0], e.what());
		// std::cerr << _argv[0] << " : " << e.what() << std::endl << std::endl;
		_options["--help"] = true;
		// _state = EXIT_FAILURE;
	}
}

bool FlagHandler::isOption(std::string option) {
	std::string opt = convertToLongOption(option);
	if (_options.find(opt) != _options.end() && _options[opt] == true) {
		return true;
	}
	return false;
}

// int FlagHandler::getState() const { return (_state); }

std::string FlagHandler::getConfigFilePath() const { return (_configFilePath); }

void FlagHandler::help() {
	std::cout << "Usage: " << _argv[0] << " [options] [config_file]" << std::endl
			  << std::endl
			  << "  Options:" << std::endl
			  << "    -h, --help\t\tDisplay this information" << std::endl
			  << "    -q, --quiet\t\tDisable the logs in the console" << std::endl
			  << "    -l, --log\t\tEnable the logs in a file" << std::endl
			  << "    -d, --debug\t\tEnable the debug mode" << std::endl
			  << std::endl
			  << "  Config file:" << std::endl
			  << "    The path to the configuration file" << std::endl
			  << "    Default: ./config/default.conf" << std::endl;
}

void FlagHandler::summary() {
	std::cout << "Summary:" << std::endl;
	std::cout << "  - Config file path: " << _configFilePath << std::endl;
	std::cout << "  - Options:" << std::endl;
	for (std::map<std::string, bool>::iterator it = _options.begin(); it != _options.end(); ++it) {
		std::cout << "    " << it->first << ": " << it->second << std::endl;
	}
}
