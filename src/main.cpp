#include "Webserv.hpp"

Server *g_server;

int ConfigParser::countFileLines = 0;

void signalHandler(int signum) {
	g_server->stop();
	// ADD and public switch to allow epoll to exit gracefully
	Logger::log(Logger::DEBUG, "interrupt signal received.", signum);
}

int main(int argc, char **argv) {
	Server server;
	g_server = &server;
	FlagHandler flags(argc, argv);
	// if (flags.isOption("--help")) {
	// 	return (flags.help(), flags.getState());
	// }
	signal(SIGINT, signalHandler);
	try {
		server.getConfigParser().parse(flags.getConfigFilePath());
		Logger::log(Logger::INFO, "Configuration file parsed");
		if (Logger::getLogDebugState()) {
			server.getConfigParser().printServers();
		}
		server.init();
		server.run();
	} catch (const std::exception &e) {
		std::cout << e.what() << std::endl;
		return (EXIT_FAILURE);
	}
	Logger::log(Logger::DEBUG, "Server stopped");
	
	return (EXIT_SUCCESS);
}
