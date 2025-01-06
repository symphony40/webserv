#ifndef REQUESTCGI_HPP
#define REQUESTCGI_HPP

#include <iostream>

#include "Utils.hpp"
#include "CgiExecutor.hpp"

#define OK		 0
#define FAIL	-1

class Request;
class CgiExecutor;

class RequestCgi {
	friend class Client;
	friend class Request;
	friend class Response;
	friend class CgiExecutor;

private:
	Request *_request;
	bool _isCGI;
	std::string _path;
	std::string _execPath;
	CgiExecutor *_cgiHandler;

	void _start();
	void _checkState();
	void _kill();

public:
	RequestCgi();
	RequestCgi(Request *request);
	RequestCgi(const RequestCgi &obj);
	~RequestCgi();

	RequestCgi &operator=(const RequestCgi &obj);
};

#include "Request.hpp"

#endif
