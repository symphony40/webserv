#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>

#include "ConfigParser.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

enum httpMethods {
	GET,
	POST,
	DELETE,
	PUT,
	UNKNOWN
};

class BlockConfigLocation {
private:
	std::string	_path;
	std::string _root;
	std::pair<int, std::string> _rewrite;
	std::string _alias;
	std::vector<std::string> _indexes;
	std::vector<httpMethods> _allowedHttpMethods;
	bool _autoindex;
	std::map<std::string, std::string> _cgiExtension;
	std::string _uploadPath;
	std::map<std::string, int> _counters;
	std::string _filename;

	bool isValidLineLocation(std::vector<std::string> &tokens, std::string& key);
	void incrementCounter(std::string const &key) { _counters[key]++; }
	void checkDoubleLine();
	void setDefaultValues();
	bool strToBool(std::string &str);
	void addAllowedMethods(std::vector<std::string> &tokens);
	void addIndexes(std::vector<std::string> &token);
	void addCgiExtension(std::vector<std::string> &token);

public:
	BlockConfigLocation(std::string filename);
	BlockConfigLocation(BlockConfigLocation const &obj);
	~BlockConfigLocation();
	BlockConfigLocation &operator=(BlockConfigLocation const &obj);

	BlockConfigLocation getLocationConfig(std::ifstream &configFile, std::string &path);
	bool getAutoIndex() const;
	bool isCgi(std::string const &path) const;
	bool isMethodAllowed(httpMethods method);
	const std::map<std::string, std::string> &getCGI() const;
	const std::pair<int, std::string> &getRewrite() const;
	const std::vector<httpMethods> &getAllowedMethods() const;
	const std::vector<std::string> &getFiles() const;
	const std::vector<std::string> &getIndexes() const;
	static httpMethods converStrToMethod(std::string const &method);
	std::string	getCgiPath(std::string const &path) const;
	std::string const &getAlias() const;
	std::string const &getPath() const;
	std::string const &getRoot() const;
	void cleanPaths();
	void printBool(std::string const& label, bool value, std::string const& trueString, std::string const& falseString);
	void printLocation();
	void printMap(std::string const& label, const std::map<std::string, std::string> &map);
	void printPair(std::string const& label, std::string const& value);
	void printVector(std::string const& label, std::vector<std::string> const &vector);
	void setAlias(std::string const &alias);
	void setAutoIndex(bool autoindex);
	void setPath(std::string const &path);
	void setRewrite(std::vector<std::string> &tokens);
	void setRoot(std::string const &root);
	void setUploadPath(std::string const &uploadPath);

};

#endif
