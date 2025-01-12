#ifndef CGIEXECUTOR_HPP
#define CGIEXECUTOR_HPP

#include <iostream>
#include <map>

#include "RequestBody.hpp"
#include "RequestCgi.hpp"

#define OK		 0
#define FAIL	-1

class RequestCgi;

class CgiExecutor {
	friend class RequestCgi;

private:
	RequestCgi *_requestCgi;
	std::map<std::string, std::string> _env;
	char** _envp;
	char** _argv; // 0: path, 1: execPath, 2: NULL
	std::map<std::string, std::string> _headers;
	std::string _tmpHeaderKey;
	std::string _tmpHeaderValue;
	RequestBody _body;
	pid_t _pid;
	int _tmpStdin;
	int _tmpStdout;
	time_t _lastActivityAt;

	void init();
	void execute();
	char** envToChar();
	char** buildArgv();

public:
	CgiExecutor(RequestCgi* requestCgi);
	CgiExecutor(CgiExecutor const &obj);
	~CgiExecutor();
	CgiExecutor &operator=(CgiExecutor const &obj);

	int	getFdOut() const;
};

#endif
