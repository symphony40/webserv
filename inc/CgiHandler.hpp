#ifndef CGIHANDLER_HPP
#define CGIHANDLER_HPP

#include <iostream>
#include <map>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctime>

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
	CgiHandler(Response* response);
	CgiHandler(const CgiHandler &src);
	~CgiHandler();

	CgiHandler &operator=(CgiHandler const &src);

	std::string getOutput() const;
	cgiHandlerState getState() const;
	static std::string getStateStr(cgiHandlerState state);

private:
	Response *_response;
	std::string _output;
	std::map<std::string, std::string> _headers;
	std::string _tmpHeaderKey;
	std::string _tmpHeaderValue;
	bool _isChunked;
	cgiHandlerState _state;

	void setState(cgiHandlerState state);
	void parse(const std::string &data);
	void parseHeaders();
	void parseHeadersKey();
	void parseHeadersValue();
	void parseBody();
	void parseChunkedBody();
	int checkHeaders();

};

#include "Response.hpp"

#endif
