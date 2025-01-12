
#ifndef UTILS_HPP
#define UTILS_HPP

#include <arpa/inet.h>
#include <cstdarg>
#include <dirent.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdint.h>
#include <string>
#include <sys/epoll.h>
#include <sys/types.h>
#include <vector>

#include "ConfigParser.hpp"
#include "ErrorPage.hpp"
#include "Logger.hpp"

#define OK		 0
#define FAIL	-1

class Utils {
public:
	static bool directoryExist(const char *path);
	static bool fileExists(std::string const &name);
	static bool isEmptyFile();
	static bool isPathWithinRoot(std::string const &root, std::string& path) ;
	static char hexToChar(char c);
	static int createFileRandomSuffix(std::string &path, int &fd);
	static int createTempFile(std::string &path, int &fd);
	static int extractPort(std::string addressPort);
	static int protectedCall(int ret, std::string msg, bool isFatal = true);
	static int urlDecode(std::string &str);
	static std::string buildPage(std::vector<std::string> files, std::string path, std::string root);
	static std::string extractAddress(std::string addressPort);
	static std::string getExtension(std::string const &path, bool includeDot = true);
	static std::string getHttpStatusMessage(int code);
	static std::string getMimeType(std::string const &path);
	static std::string intToHex(ssize_t num);
	static std::string intToString(int num);
	static std::string listDirectory(std::string path, std::string root);
	static std::string toLowerCase(std::string const &str);
	static std::string trimLine(std::string &line);
	static std::string uint64ToString(u_int64_t num);
	static std::string uintToString(unsigned int num);
	static std::string ullToStr(unsigned long long num);
	static std::vector<std::string> split(std::string s, std::string delimiter);
	static unsigned long long strToUll(std::string clientMaxBodySize);
	static void cleanPath(std::string &path);
	static void printMessage(std::ostream &os, const char *msg, ...);
	static void socketEpollAdd(int epollFD, int sockFD, uint32_t flags);
	static void socketEpollDelete(int epollFD, int sockFD);
	static void socketEpollModify(int epollFD, int sockFD, uint32_t flags);
};

// EXCEPTIONS

class IntException : public std::exception {
private:
	int _code;

public:
	IntException(int code) : _code(code) {}
	virtual ~IntException() throw() {}

	int code() const;
};

class ChildProcessException : public std::exception {};


#endif
