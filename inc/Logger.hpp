#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#define CYAN "\001\033[36m\002"
#define GREEN "\001\033[32m\002"
#define MAGENTA "\001\033[35m\002"
#define RED "\001\033[31m\002"
#define RESET "\001\033[0m\002"
#define YELLOW "\001\033[33m\002"

#define REQUEST_FLAGS EPOLLIN | EPOLLRDHUP | EPOLLERR 
#define RESPONSE_FLAGS EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLOUT 
#define MAX_EVENTS 100

#define DEFAULT_LOG_DEBUG_STATE false
#define DEFAULT_LOG_FILE_STATE false
#define DEFAULT_LOG_STATE true

class Logger {
public:
	enum LogLevel { FATAL, ERROR, WARNING, INFO, TRACE, DEBUG, };

	static bool getLogDebugState();
	static bool getLogFileState();
	static bool getLogState();
	static std::string getLogFileName();
	static std::string getLogLevelColor(LogLevel level);
	static std::string getLogLevelStr(LogLevel level);
	static void log(LogLevel level, const char *msg, ...);
	static void setLogDebugState(bool state);
	static void setLogFileState(bool state);
	static void setLogState(bool state);

private:
	static bool _logDebugState;
	static bool _logFileState;
	static bool _logState;
	static std::map<Logger::LogLevel, std::string> _logLevelColor;
	static std::map<Logger::LogLevel, std::string> _logLevelString;
	static std::string _logFileName;

	static std::map<Logger::LogLevel, std::string> generateLogLevelColor();
	static std::map<Logger::LogLevel, std::string> generateLogLevelString();
	static std::string format(Logger::LogLevel level, char const *msg, std::string time, bool colored = true);
	static std::string generateLogFileName();
	static void printLog(Logger::LogLevel level, char const *msg, std::string time);
	static void writeLogInFile(LogLevel level, char const *msg, std::string time);

};

#endif
