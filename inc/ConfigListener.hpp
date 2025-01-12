
#ifndef CONFIGLISTENER_HPP
#define CONFIGLISTENER_HPP

#include <iostream>
#include <vector>

#include "Logger.hpp"
#include "Utils.hpp"

class ConfigListener {
private:
 	std::string _address;
	unsigned int _port;
	std::string _addressPortCombo;

	bool checkAddressPort();
	bool isAddressOutOfRange(std::string str, int j, int i);

public:
	ConfigListener();
	ConfigListener(std::string token);
	~ConfigListener();

	std::string const &getAddress() const;
	unsigned int getPort() const;
	std::string const getAddressPortCombined() const;

};

#endif
