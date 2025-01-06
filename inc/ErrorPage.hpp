#ifndef ERRORPAGE_HPP
#define ERRORPAGE_HPP

#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Utils.hpp"

class ErrorPage {
public:
	static std::string getPage(int statusCode, std::map<int, std::string> errorPagesCustom = std::map<int, std::string>());
	static std::string getErrorPagesCustom(int statusCode, std::map<int, std::string> errorPagesCustom);
};

#endif
