#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <sys/stat.h>
#include <sys/epoll.h>

#include "Request.hpp"
#include "Client.hpp"
#include "BlocServer.hpp"
#include "Utils.hpp"
#include "BlocLocation.hpp"
#include "ErrorPage.hpp"
#include "CgiHandler.hpp"

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
	std::vector<std::string> getAllPathsLocation();
	CgiHandler &getCgiHandler();
	void setError(int code, bool generatePage = true);

private:
	Request *_request;
	CgiHandler _cgiHandler;
	std::string _response;
	responseState _state;
	int _fileFd;

	bool isLargeFile(const std::string& path);
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
	void prepareChunkedResponse(const std::string &path);
	void prepareStandardResponse(const std::string &path);
	void setHeaderChunked(const std::string &path);
	void setState(responseState state);
};

#endif
