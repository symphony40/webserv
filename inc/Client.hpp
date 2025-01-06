#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <sys/socket.h>
# include <netinet/in.h>
# include <ctime>

# include "Utils.hpp"
# include "Request.hpp"
# include "Socket.hpp"
# include "Response.hpp"

# define CLIENT_READ_BUFFER_SIZE 8192  // 4096

class Request;
class Response;


class Client {
private:
	int _fd;
	Socket *_socket;
	Request *_request;
	Response *_response;
	time_t _lastActivity;

public:
	Client(int fd, Socket* socket);
	~Client();

	void handleRequest();
	void handleResponse(int epollFD);
	void reset();
	int getFd() const;
	Request *getRequest() const;
	Socket *getSocket() const;
	Response *getResponse() const;
	time_t getLastActivity() const;
	void updateLastActivity();
	void checkCgi();

	class DisconnectedException : public std::exception	{
		public:
			virtual const char *what() const throw() {
				return "Client disconnected";
			}
	};
};

#endif
