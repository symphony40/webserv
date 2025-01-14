#include "main.hpp"

Server *g_server;

int ConfigParser::fileLineCount = 0;

void signalHandler(int signum) {
	g_server->stop();
	Logger::log(Logger::DEBUG, "interrupt signal received.", signum);
}

int main(int argc, char **argv) {
	Server server;
	g_server = &server;
	FlagHandler flags(argc, argv);
	signal(SIGINT, signalHandler);
	try {
		server.getConfigParser().parse(flags.getConfigFilePath());
		Logger::log(Logger::INFO, "Configuration file parsed");
		if (Logger::getLogDebugState()) {
			server.getConfigParser().printServers();
		}
		server.init();
		server.run();
	} catch (std::exception const &e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	Logger::log(Logger::DEBUG, "Server stopped");
	
	return EXIT_SUCCESS;
}
