#ifndef ERRORPAGE_HPP
#define ERRORPAGE_HPP

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "Utils.hpp"

class ErrorPage {
public:
	static std::string getPage(int statusCode, std::map<int, std::string> errorPagesCustom = std::map<int, std::string>());
	static std::string getErrorPagesCustom(int statusCode, std::map<int, std::string> errorPagesCustom);
};

#endif
