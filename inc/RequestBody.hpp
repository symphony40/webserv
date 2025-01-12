#ifndef REQUESTBODY_HPP
#define REQUESTBODY_HPP

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>

#include "Logger.hpp"
#include "Utils.hpp"

#define OK		 0
#define FAIL	-1

class RequestBody {
	friend class Request;
	friend class CgiHandler;
	friend class CgiExecutor;

private:
	std::string _path;
	int _fd;
	bool _isTemp;
	unsigned long long _size;

	int	writeData(std::string const &data);

public:
	RequestBody();
	RequestBody(bool isTemp);
	RequestBody(RequestBody const &obj);
	~RequestBody();

	RequestBody &operator=(RequestBody const &obj);

	std::string getPath() const;
	int getFd() const;
	bool isTemp() const;
	unsigned long long getSize() const;

};

#endif
