#include "ListenConfig.hpp"

ListenConfig::ListenConfig() {}

ListenConfig::ListenConfig(std::string token) {
	std::vector<std::string> ipAddrPort = Utils::split(token, ":");

	if (ipAddrPort.size() == 1) {
		if (ipAddrPort[0].find(".") != std::string::npos) {
			_address = ipAddrPort[0];
			_port = 80;
		} else {
			_address = "0.0.0.0";
			_port = std::atoi(ipAddrPort[0].c_str());
		}
	} else if (ipAddrPort.size() == 2) {
		if (ipAddrPort[0].empty() || ipAddrPort[1].empty()) {
			Logger::log(Logger::FATAL, "Invalid Ip value: %s", token.c_str());
		}
		_address = ipAddrPort[0];
		_port = std::atoi(ipAddrPort[1].c_str());
	} else {
		Logger::log(Logger::FATAL, "Invalid listen value: %s", token.c_str());
	}
	_addressPortCombo = _address + ":" + Utils::uintToString(_port);
	if (!checkAddressPort()) {
		Logger::log(Logger::FATAL, "Invalid Ip/Port value: %s", token.c_str());
	}
}

ListenConfig::~ListenConfig() {}


bool ListenConfig::isAddressOutOfRange(std::string str, int start, int i) {
	std::string octet = str.substr(start, i);
	return (octet < "0" || octet > "255");
}

bool ListenConfig::checkAddressPort() {
	int	i = 0;
	int	j = 0;
	std::string str = _addressPortCombo;

	for (int p = 0; p < 3; p++) {
		while (isdigit(str[i + j])) {
			i++;
		}
		if (isAddressOutOfRange(str, j, i)) {
			return false;
		}
		if (i == 0 || i > 3 || str[j + i++] != '.') {
			return false;
		}
		j += i;
		i = 0;
	}
	while (isdigit(str[i + j])) {
		i++;
	}
	if (isAddressOutOfRange(str, j, i)) {
		return false;
	}
	if (i == 0 || i > 3 || str[j + i++] != ':') {
		return false;
	}
	if (_port > UINT16_MAX) {
		return false;
	}
	return true;
}

std::string const &ListenConfig::getAddress() const { return _address; }

unsigned int ListenConfig::getPort() const { return _port; }

std::string const ListenConfig::getAddressPortCombined() const { return _addressPortCombo; }
