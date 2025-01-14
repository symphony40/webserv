#include "Logger.hpp"
#include <stdarg.h>

bool Logger::_logState = DEFAULT_LOG_STATE;
bool Logger::_logFileState = DEFAULT_LOG_FILE_STATE;
bool Logger::_logDebugState = DEFAULT_LOG_DEBUG_STATE;
std::string Logger::_logFileName = Logger::generateLogFileName();
std::map<Logger::LogLevel, std::string> Logger::_logLevelString = Logger::generateLogLevelString();
std::map<Logger::LogLevel, std::string> Logger::_logLevelColor = Logger::generateLogLevelColor();

std::string Logger::generateLogFileName() {
	std::time_t t = std::time(NULL);
	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "webserv_%Y-%m-%d_%H-%M-%S.log", std::localtime(&t));
	return (std::string(buffer));
}

std::map<Logger::LogLevel, std::string> Logger::generateLogLevelString() {
	std::map<Logger::LogLevel, std::string> logLevelString;
	logLevelString[Logger::FATAL] = "FATAL";
	logLevelString[Logger::ERROR] = "ERROR";
	logLevelString[Logger::WARNING] = "WARNING";
	logLevelString[Logger::INFO] = "INFO";
	logLevelString[Logger::TRACE] = "TRACE";
	logLevelString[Logger::DEBUG] = "DEBUG";
	return logLevelString;
}

std::map<Logger::LogLevel, std::string> Logger::generateLogLevelColor() {
	std::map<Logger::LogLevel, std::string> _logLevelColor;
	_logLevelColor[Logger::FATAL] = RED;
	_logLevelColor[Logger::ERROR] = RED;
	_logLevelColor[Logger::WARNING] = YELLOW;
	_logLevelColor[Logger::INFO] = GREEN;
	_logLevelColor[Logger::TRACE] = MAGENTA;
	_logLevelColor[Logger::DEBUG] = CYAN;
	return _logLevelColor;
}

std::string Logger::format(Logger::LogLevel level, char const *msg, std::string time, bool colored) {
	std::string formatted;
	if (colored == true) {
		formatted += Logger::getLogLevelColor(level);
	}
	formatted += "[" + Logger::getLogLevelStr(level) + "]\t";
	formatted += time + "  " + msg;
	if ((level == Logger::FATAL) && errno != 0) {
		formatted += ": " + static_cast<std::string>(std::strerror(errno));
	}
	if (colored == true) {
		formatted += RESET;
	}
	return formatted;
}

void Logger::printLog(Logger::LogLevel level, const char *msg, std::string time) {
	std::cout << Logger::format(level, msg, time) << std::endl;
}

void Logger::writeLogInFile(Logger::LogLevel level, const char *msg, std::string time) {
	if (mkdir("logs", 0777) == -1 && errno != EEXIST) {
		std::cerr << "Error: " << std::strerror(errno) << std::endl;
		return;
	}
	int file = open(("logs/" + Logger::getLogFileName()).c_str(), O_CREAT | O_WRONLY | O_APPEND, 0666);
	std::string log = Logger::format(level, msg, time, false);
	write(file, log.c_str(), log.size());
	write(file, "\n", 1);
	close(file);
}

void Logger::log(Logger::LogLevel level, const char *msg, ...) {
	
	if (Logger::getLogState() == false || (level == Logger::DEBUG && Logger::getLogDebugState() == false)) {
		return;
	}
	int const initialBufferSize = 1024;
	std::vector<char> buffer(initialBufferSize);
	va_list args;
	va_start(args, msg);
	int size = vsnprintf(buffer.data(), buffer.size(), msg, args);
	va_end(args);
	if (size < 0) {
		return;
	}
	while (size >= static_cast<int>(buffer.size())) {
		buffer.resize(buffer.size() * 2);
		va_start(args, msg);
		size = vsnprintf(buffer.data(), buffer.size(), msg, args);
		va_end(args);
	}
	std::time_t t = std::time(NULL);
	char timeBuffer[80];
	std::strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", std::localtime(&t));
	Logger::printLog(level, buffer.data(), timeBuffer);
	if (Logger::getLogFileState() == true) {
		Logger::writeLogInFile(level, buffer.data(), timeBuffer);
	}
	if (level == Logger::FATAL) {
		throw std::runtime_error(buffer.data());
	}
}

void Logger::setLogState(bool state) { Logger::_logState = state; } 

void Logger::setLogFileState(bool state) { Logger::_logFileState = state; }

void Logger::setLogDebugState(bool state) { Logger::_logDebugState = state; }

bool Logger::getLogState() { return Logger::_logState; }

bool Logger::getLogFileState() { return Logger::_logFileState; }

bool Logger::getLogDebugState() { return Logger::_logDebugState; }

std::string Logger::getLogFileName() { return Logger::_logFileName; }

std::string Logger::getLogLevelStr(Logger::LogLevel level) { return Logger::_logLevelString[level]; }

std::string Logger::getLogLevelColor(Logger::LogLevel level) { return Logger::_logLevelColor[level]; }
