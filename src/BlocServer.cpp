#include "BlocServer.hpp"

// ============ GENERAL ============

BlocServer::BlocServer(void) : _clientMaxBodySize(CLIENT_MAX_BODY_SIZE), _filename("")
{
	_counterView["root"] = 0;
	_counterView["clientMaxBodySize"] = 0;
}

BlocServer::BlocServer(std::string filename) : _clientMaxBodySize(CLIENT_MAX_BODY_SIZE), _filename(filename)
{
	_counterView["root"] = 0;
	_counterView["clientMaxBodySize"] = 0;
}

BlocServer::BlocServer(const BlocServer &other)
{
	*this = other;
}

BlocServer::~BlocServer(void){}

BlocServer &BlocServer::operator=(const BlocServer &other)
{
	if (this != &other)
	{
		_listens = other._listens;
		_serverNames = other._serverNames;
		_indexes = other._indexes;
		_root = other._root;
		_clientMaxBodySize = other._clientMaxBodySize;
		_locations = other._locations;
		_errorPages = other._errorPages;
		_filename = other._filename;
		_counterView = other._counterView;
	}
	return *this;
}

// ============ UTILS ============
void BlocServer::setClientMaxBodySize(std::string clientMaxBodySize)
{
	_clientMaxBodySize = Utils::strToUll(clientMaxBodySize);
	_counterView["clientMaxBodySize"]++;
}


bool BlocServer::isStartBlocLocation(std::vector<std::string>& tokens)
{
	return (tokens.size() == 3 && tokens[0] == "location" && tokens[2] == "{");
}

void BlocServer::addListen(std::string &token)
{
	ListenConfig listen(token);
	
	if (_listens.find(listen.getAddressPortCombined()) != _listens.end())
		Logger::log(Logger::FATAL, "Duplicate listen in server context: %s", token.c_str());

	_listens[listen.getAddressPortCombined()] = listen;
}

void BlocServer::addServerName(std::vector<std::string>& token){
	for (size_t i = 1; i < token.size(); i++){
		if (std::find(_serverNames.begin(), _serverNames.end(), token[i]) == _serverNames.end())
			_serverNames.push_back(token[i]);
	}
}

void BlocServer::addIndexes(std::vector<std::string>& token){
	for (size_t i = 1; i < token.size(); i++){
		if (std::find(_indexes.begin(), _indexes.end(), token[i]) == _indexes.end())
			_indexes.push_back(token[i]);
	}
}

void BlocServer::addErrorPages(int errorCode, std::string file)
{ 
	if (errorCode < 400 || errorCode > 599)
		Logger::log(Logger::FATAL, "Invalid error code: %d in file %s:%d", errorCode, _filename.c_str(), ConfigParser::countFileLines);
	_errorPages[errorCode] = file; 
}


/**
 * fonction qui check si un server_name de otherNames n'est
 * pas egal a un serverName de _serverNames de l'instance actuel
*/
bool BlocServer::isServerNamePresent(std::vector<std::string>& otherNames){

	for (size_t i = 0; i < otherNames.size(); i++)
	{
		for (size_t j = 0; j < _serverNames.size(); j++)
		{
			if (otherNames[i] == _serverNames[j]){
				otherNames.clear();
				otherNames.push_back(_serverNames[j]);
				otherNames.push_back(_listens.begin()->first);
				return (true);
			}
		}
	}
	return (false);
}


/**
 * @brief remove all trailing slashes from paths
 * trailing ca veut dire a la fin
 */
void BlocServer::cleanPaths()
{
	// clean root path
	if (!_root.empty() && _root != "/" && _root[_root.size() - 1] == '/')
		_root.erase(_root.size() - 1);

	// clean all error pages path
	for (std::map<int, std::string>::iterator it = _errorPages.begin(); it != _errorPages.end(); ++it){
		if (it->second != "/" && it->second[it->second.size() - 1] == '/')
			it->second.erase(it->second.size() - 1);
	}

	// clean all Location path
	std::vector<BlocLocation>::iterator it;
	for (it = _locations.begin(); it != _locations.end(); ++it){
		it->cleanPaths();
	}
}
// ============ CHECKER ============


void BlocServer::checkDoubleLine()
{
	std::map<std::string, int>::iterator it;

	for (it = _counterView.begin(); it != _counterView.end(); ++it)
		if (it->second > 1)
			Logger::log(Logger::FATAL, "Duplicate line in server context: %s", it->first.c_str());
}


void BlocServer::checkDoubleLocation()
{
	std::vector<BlocLocation>::iterator it;
	std::vector<BlocLocation>::iterator it2;

	for (it = _locations.begin(); it != _locations.end(); ++it)
	{
		for (it2 = it + 1; it2 != _locations.end(); ++it2)
		{
			if (it->getPath() == it2->getPath())
				Logger::log(Logger::FATAL, "Duplicate location: \"%s\" in file: %s", it->getPath().c_str(), _filename.c_str());
		}
	}
}



// ============ PARSING ============

/**
 * @brief This function sets the default values for the server object.
 * At the all end
 * 
 */
void BlocServer::setDefaultValue()
{
	if (_listens.empty()){
		ListenConfig listen("0.0.0.0:1234");
		_listens["0.0.0.0:1234"] = listen;
	}
	if (_root.empty())
		_root = "./www/main";
	if (_indexes.empty())
		_indexes.push_back("index.html");
}

/**
 * @brief This function checks if a line in the configuration file is a good directive in BlocServer bloc (server_name par ex)
 * It updates the server object with the corresponding values if the line is valid.
 * 
 * @param tokens  A vector of strings containing the tokens of the line.
 * @param key The key of the line. (the first token)
 */
bool BlocServer::isValidLineServer(std::vector<std::string>& tokens, std::string& key, std::ifstream &configFile){
	if (tokens.size() < 2)
		return false;
	if (isStartBlocLocation(tokens)){
		BlocLocation location(_filename);
		addLocation(location.getLocationConfig(configFile, tokens[1]));
	}
	else if (key == "listen" && tokens.size() == 2)
		addListen(tokens[1]);
	else if (key == "server_name")
		addServerName(tokens);
	else if (key == "index")
		addIndexes(tokens);
	else if (key == "root" && tokens.size() == 2)
		setRoot(tokens[1]);
	else if (key == "client_max_body_size" && tokens.size() == 2)
		setClientMaxBodySize(tokens[1]);
	else if (key == "error_page" && tokens.size() == 3){
		addErrorPages(std::atoi(tokens[1].c_str()), tokens[2]);
	}else
		return (false);
	return (true);
}


/**
 * @brief parse a server bloc and if encounter a location bloc, it call getLocationConfig() to parse it
 * 
 * @param configFile 
 * @return BlocServer 
 */
BlocServer BlocServer::getServerConfig(std::ifstream &configFile)
{
	std::string line;
	std::vector<std::string> tokens;
	std::string key;
	bool isCloseServer = false;

	while (std::getline(configFile, line)){
		ConfigParser::countFileLines++;
		line = Utils::trimLine(line);
		if (line.empty() || line[0] == '#')
			continue;
		tokens = Utils::split(line, " ");
		key = tokens[0];
		if (key[0] == '}' && key.size() == 1 && tokens.size() == 1){
			isCloseServer = true;
			break;
		}
		else if (isValidLineServer(tokens, key, configFile))
			continue ;
		else
			Logger::log(Logger::FATAL, "Invalid line: \"%s\" in file: %s:%d", line.c_str(), _filename.c_str(), ConfigParser::countFileLines);
	}
	if (isCloseServer == false && !Utils::isEmptyFile())
		Logger::log(Logger::FATAL, "Missing } in file %s:%d", _filename.c_str(), ConfigParser::countFileLines);
	checkDoubleLine();
	setDefaultValue();
	checkDoubleLocation();
	cleanPaths();
	return (*this);
}

// =========== FINDERS ===========

/**
 * @brief find a location by uri
 * 
 * @param uri 
 * @return BlocLocation* 
 */
BlocLocation* BlocServer::findLocation(const std::string &uri)
{
	for (std::vector<BlocLocation>::iterator it = _locations.begin(); it != _locations.end(); ++it)
	{
		if (uri.find(it->getPath()) == 0)
			return &(*it);
	}
	return NULL;
}




// ============ PRINT ============


void BlocServer::printPair(const std::string& label, const std::string& value)
{
    std::cout << std::left << label << ": " << (value.empty() ? "none" : value) << std::endl;
}

void BlocServer::printInt(const std::string& label, int value)
{
    std::cout << std::left << label << ": " << (value == -1 ? "none" : Utils::intToString(value)) << std::endl;
}

void BlocServer::printVector(const std::string& label, const std::vector<std::string>& vec)
{
    std::cout << std::left << label << ": " << (vec.empty() ? "none" : "") << std::endl;
    for (std::vector<std::string>::const_iterator it = vec.begin(); it != vec.end(); ++it)
        std::cout << "\t- " << *it << std::endl;
}

void BlocServer::printMap(const std::string& label, const std::map<int, std::string>& map)
{
    std::cout << std::left << label << ": " << (map.empty() ? "none" : "") << std::endl;
    for (std::map<int, std::string>::const_iterator it = map.begin(); it != map.end(); ++it)
        std::cout << "\t- " << it->first << ": " << it->second << std::endl;
}

void BlocServer::printListens()
{
    std::cout << std::left << "Listens" << ": " << (_listens.empty() ? "none" : "") << std::endl;
    for (std::map<std::string, ListenConfig>::const_iterator it = _listens.begin(); it != _listens.end(); ++it)
        std::cout << "\t- " << it->second.getAddress() << ":" << it->second.getPort() << std::endl;
}

void BlocServer::printServer(void)
{
    printVector("Server names", _serverNames);
    printListens();
    printVector("Indexes", _indexes);
    printPair("Root", _root);
    // printInt("Client max body size", _clientMaxBodySize);
	std::cout << "Client max body size: " << Utils::ullToStr(_clientMaxBodySize) << std::endl;
    printMap("Error pages", _errorPages);

    if (_locations.empty()){
        std::cout << std::setw(25) << std::left << "Locations" << ": none" << std::endl;
    }
    else
    {
        int i = 0;
        for (std::vector<BlocLocation>::iterator it = _locations.begin(); it != _locations.end(); ++it){
            std::cout << "\n-- LOCATION " << ++i << " --" << std::endl;
            it->printLocation();
        }
    }
}
