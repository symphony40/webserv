#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <ctime>
#include <iostream>
#include <map>
#include <sys/types.h>
#include <sys/wait.h>

#include "Utils.hpp"

#define OK		 0
#define FAIL	-1

class Response;

class CgiHandler {
	friend class Response;

public:
	enum cgiHandlerState {
		INIT,
		HEADERS_PARSE_KEY,
		HEADERS_PARSE_VALUE,
		HEADERS_PARSE_END,
		HEADERS_END,
		BODY,
		FINISH
	};
	CgiHandler(Response *response);
	CgiHandler(CgiHandler const &obj);
	~CgiHandler();
	CgiHandler &operator=(CgiHandler const &obj);

	static std::string getState(cgiHandlerState state);
	std::string getOutput() const;
	cgiHandlerState getState() const;

private:
	Response *_response;
	std::string _output;
	std::map<std::string, std::string> _headers;
	std::string _tmpHeaderKey;
	std::string _tmpHeaderValue;
	bool _isChunked;
	cgiHandlerState _state;

	int checkHeaders();
	void parse(std::string const &data);
	void parseBody();
	void parseChunkedBody();
	void parseHeaders();
	void parseHeadersKey();
	void parseHeadersValue();
	void setState(cgiHandlerState state);

};

#include "Response.hpp"

#endif
