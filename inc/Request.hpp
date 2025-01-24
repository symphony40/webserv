#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <algorithm>
#include <map>
#include <sstream>
#include <string>

#include "Logger.hpp"
#include "Client.hpp"
#include "ConfigParser.hpp"
#include "RequestCgi.hpp"
#include "RequestBody.hpp"

#define FAIL -1
#define OK 0
#define REQUEST_DEFAULT_BODY_TIMEOUT 3600
#define REQUEST_DEFAULT_CGI_TIMEOUT 3
#define REQUEST_DEFAULT_HEADER_TIMEOUT 10
#define REQUEST_DEFAULT_STATE_CODE 200
#define REQUEST_DEFAULT_UPLOAD_PATH "./w3/upload/"

class Client;
class RequestBody;

class Request {	
	friend class Client;
	friend class RequestCgi;
	friend class RequestBody;
	friend class CgiHandler;
	friend class CgiExecutor;
	friend class Response;
	
public:
	enum parsingState {
		INIT,
		REQUEST_LINE_METHOD,
		REQUEST_LINE_URI,
		REQUEST_LINE_HTTP_VERSION,
		REQUEST_LINE_END,
		HEADERS_INIT,
		HEADERS_PARSE_KEY,
		HEADERS_PARSE_VALUE,
		HEADERS_PARSE_END,
		HEADERS_END,
		BODY_INIT,
		BODY_PROCESS,
		BODY_END,
		CGI_INIT,
		CGI_PROCESS,
		CGI_END,
		FINISH
	};
	Request(Client *client);
	Request(Request const &obj);
	~Request();
	Request &operator=(Request const &obj);

	BlockConfigRoute *getRoute() const;
	BlockConfigServer *getServer() const;
	bool isCgi() const;
	bool isChunked() const;
	Client *getClient() const;
	int getChunkSize() const;
	int getStateCode() const;
	parsingState getState() const;
	RequestBody &getBody();
	RequestCgi &getCgi();
	static std::string getParseStateString(parsingState state);
	std::map<std::string, std::string> getHeaders() const;
	std::string getHttpVersion() const;
	std::string getMethod() const;
	std::string getPath() const;
	std::string getQuery() const;
	std::string getRawRequest() const;
	std::string getUri() const;
	time_t getTimeout() const;
	unsigned long long getContentLength() const;
	void checkTimeout();
	void parse(std::string const &rawRequest);
	void setCgi(bool isCgi, std::string const &path, std::string const &execPath) { _cgi._isCGI = isCgi; _cgi._path = path; _cgi._execPath = execPath; }
	void setError(int code);
	void setStateCode(int code);
	void setTimeout(int timeout);
	void setTimeout(time_t timeout);

private:
	Client *_client;
	BlockConfigServer *_serverBlock;
	BlockConfigRoute *_routeBlock;
	std::string _rawRequest;
	std::string _httpMethod;
	std::string _uri;
	std::string _path;
	std::string _query;
	std::string _httpVersion;
	RequestBody _body;
	std::map<std::string, std::string> _headers;
	std::string _tmpHeaderKey;
	std::string _tmpHeaderValue;
	bool _isChunked;
	RequestCgi _cgi;
	unsigned long long _contentLength;
	int _chunkSize;
	time_t _timeout;
	parsingState _state;
	int _stateCode;

	int checkCgi();
	int checkHttpMethod();
	int checkPathsMatch(std::string const &path, std::string const &parentPath);
	int checkTransferEncoding();
	int findRoute();
	int findServer();
	int processUri();
	int checkClientMaxBodySize();
	std::vector<std::string> getAllPathsInRoute();
	void defineBodyDestination();
	void initServer();
	void initTimeout();
	void parseBody();
	void parseChunkedBody();
	void parseHeaders();
	void parseHeadersKey();
	void parseHeadersValue();
	void parseHttpVersion();
	void parseMethod();
	void parseRequestLine();
	void parseUri();
	void setHeaderState();
	void setState(parsingState state);
	
};

#endif
