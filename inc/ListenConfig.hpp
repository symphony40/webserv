
#ifndef LISTENCONFIG_HPP
#define LISTENCONFIG_HPP

#include <iostream>
#include <vector>

#include "Utils.hpp"
#include "Logger.hpp"

class ListenConfig {
private:
 	std::string _address;
	unsigned int _port;
	std::string _addressPortCombo;

	bool checkAddressPort();
	bool isAddressOutOfRange(std::string str, int j, int i);

public:
	ListenConfig();
	ListenConfig(std::string token);
	~ListenConfig();

	std::string const &getAddress() const;
	unsigned int getPort() const;
	const std::string getAddressPortCombined() const;

};

#endif
