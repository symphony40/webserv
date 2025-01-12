#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>

#include "BlockConfigLocation.hpp"
#include "BlockConfigServer.hpp"
#include "CgiHandler.hpp"
#include "Client.hpp"
#include "ErrorPage.hpp"
#include "Request.hpp"
#include "Utils.hpp"

#define RESPONSE_READ_BUFFER_SIZE 	4096
#define THRESHOLD_LARGE_FILE 		100000
#define OK		 					0
#define FAIL						-1

class Client;
class Request;

class Response {
	friend class CgiHandler;
public:
	enum responseState { INIT, PROCESS, CHUNK, FINISH };
	Response(Client *client);
	~Response();

	int getState() const;
	std::string getResponse() const;
	size_t	getResponseSize() const;
	int generateResponse(int epollFD);
	std::vector<std::string> getAllPathLocations();
	CgiHandler &getCgiHandler();
	void setError(int code, bool generatePage = true);

private:
	Request *_request;
	CgiHandler _cgiHandler;
	std::string _response;
	responseState _state;
	int _fileFd;

	bool isLargeFile(std::string const& path);
	bool isRedirect();
	int	handleCgi();
	std::string findGoodPath(std::vector<std::string> allPaths);
	std::vector<std::string> getAllPathsServer();
	void handleDeleteRequest();
	void handleGetRequest();
	void handlePostRequest();
	void handlePutRequest();
	void handleLocation();
	void handleNotFound(std::string directoryToCheck);
	void handleServer();
	void prepareChunkedResponse(std::string const &path);
	void prepareStandardResponse(std::string const &path);
	void setHeaderChunked(std::string const &path);
	void setState(responseState state);
};

#endif
