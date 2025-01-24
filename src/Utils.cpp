#include "main.hpp"
#include "Utils.hpp"

int Utils::createTempFile(std::string &path, int &fd) {
	path = "/tmp/webserv_XXXXXX";
	std::vector<char> tempPath(path.begin(), path.end());
	tempPath.push_back('\0');
	fd = mkstemp(&tempPath[0]);
	if (fd == FAIL) {
		Logger::log(Logger::ERROR, "[Utils::createTempFile] Failed to create temporary file");
		return FAIL;
	}
	path.assign(tempPath.begin(), tempPath.end() - 1);
	return OK;
}

int Utils::createFileRandomSuffix(std::string &path, int &fd) {
	path += "XXXXXX";
	std::vector<char> tempPath(path.begin(), path.end());
	tempPath.push_back('\0');
	fd = mkstemp(&tempPath[0]);
	if (fd == FAIL) {
		Logger::log(Logger::ERROR, "[Utils::createTempFile] Failed to create temporary file");
		return FAIL;
	}
	path.assign(tempPath.begin(), tempPath.end() - 1);
	return OK;
}

int Utils::urlDecode(std::string &url) {
	size_t i = -1;
	while (++i < url.size()) {
		if (url[i] == '%') {
			if (i + 2 >= url.size()) {
				return FAIL;
			}
			char c = hexToChar(url[i + 1]) * 16 + hexToChar(url[i + 2]);
			url[i] = c;
			url.erase(i + 1, 2);
		}
	}
	return OK;
}

char Utils::hexToChar(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return OK;
}

unsigned long long Utils::strToUll(std::string clientMaxBodySize) {
	unsigned long long size = BODY_SIZE_MAX;
	std::stringstream ss(clientMaxBodySize);
	ss >> size;
	if (ss.fail() || !ss.eof())	{
		return BODY_SIZE_MAX;
	}
	return size;
}

std::string Utils::ullToStr(unsigned long long num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

void Utils::printMessage(std::ostream &os, const char *msg, ...) {
	const int initialBufferSize = 1024;
	std::vector<char> buffer(initialBufferSize);
	va_list args;
	va_start(args, msg);
	int size = vsnprintf(buffer.data(), initialBufferSize, msg, args);
	va_end(args);
	if (size < 0) {
		return;
	}
	while (size >= static_cast<int>(buffer.size()))	{
		buffer.resize(buffer.size() * 2);
		va_start(args, msg);
		size = vsnprintf(buffer.data(), buffer.size(), msg, args);
		va_end(args);
	}
	os << buffer.data() << std::endl;
}

bool Utils::directoryExist(char const *path) {
	struct stat info;
	if (stat(path, &info) != 0)	{
		return false;
	}
	return (info.st_mode & S_IFDIR) != 0;
}

bool Utils::fileExists(std::string const& path) {
    struct stat buffer;
    return stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode);
}

std::string Utils::trimLine(std::string &line) {
	std::string const whiteSpace = " \t\n\r\f\v";
	std::string res;
	int start = 0;
	while (whiteSpace.find(line[start]) != std::string::npos) {
		start++;
	}
	int end = line.size() - 1;
	while (end >= 0 && whiteSpace.find(line[end]) != std::string::npos) {
		end--;
	}
	for (int i = start; i <= end; i++) {
		res.push_back(line[i]);
	}
	return res;
}

std::vector<std::string> Utils::split(std::string s, std::string delimiter) {
	size_t pos = 0;
	std::string token;
	std::vector<std::string> res;
	s = Utils::trimLine(s);
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		res.push_back(token);
		s.erase(0, pos + delimiter.length());
		s = Utils::trimLine(s);
	}
	res.push_back(s);
	return res;
}

std::string Utils::uintToString(unsigned int num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

std::string Utils::intToString(int num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

int Utils::tryCall(int code, std::string message, bool isFatal) {
	if (code < 0) {
		if (isFatal) {
			if (g_server->getState() != SERVER_STATE_STOP) {
				Logger::log(Logger::FATAL, message.c_str());
			}
		} else {
			Logger::log(Logger::ERROR, message.c_str());
		}
	}
	return code;
}

std::string Utils::extractAddress(std::string addressPort) {
	return addressPort.substr(0, addressPort.find(":"));
}

int Utils::extractPort(std::string addressPort) {
	return std::atoi(addressPort.substr(addressPort.find(":") + 1).c_str());
}
	
bool Utils::isEmptyFile() {
	return ConfigParser::fileLineCount == 0;
}

std::string Utils::intToHex(ssize_t num) {
    std::stringstream stream;
    stream << std::hex << num;
    return stream.str();
}

std::string Utils::getExtension(std::string const &path, bool includeDot) {
	std::string ext;
	std::string::size_type i = path.rfind('.');
	if (i != std::string::npos) {
		if (includeDot) {
			ext = path.substr(i);
		} else {
			ext = path.substr(i + 1);
		}
	}
	if ((includeDot && ext.size() <= 1) || (!includeDot && ext.empty()) || (ext.find('/') != std::string::npos)) {
		return "";
	}
	return ext;
}

void Utils::socketEpollAdd(int epollFD, int sockFD, uint32_t flags) {
	epoll_event ev;
	ev.events = flags;
	ev.data.fd = sockFD;
	Utils::tryCall(epoll_ctl(epollFD, EPOLL_CTL_ADD, sockFD, &ev), "Error with epoll_ctl function", false);
}

void Utils::socketEpollModify(int epollFD, int sockFD, uint32_t flags) {
	epoll_event ev;
	ev.events = flags;
	ev.data.fd = sockFD;
	Utils::tryCall(epoll_ctl(epollFD, EPOLL_CTL_MOD, sockFD, &ev), "Error with epoll_ctl function", false);
}

void Utils::socketEpollDelete(int epollFD, int sockFD) {
	epoll_event ev;
	ev.data.fd = sockFD;
	Utils::tryCall(epoll_ctl(epollFD, EPOLL_CTL_DEL, sockFD, &ev), "Error with epoll_ctl function", false);
}

std::string Utils::getHttpStatusMessage(int code) {
	switch (code) {
		case 200: return "OK";
		case 201: return "Created";
		case 202: return "Accepted";
		case 204: return "No Content";
		case 301: return "Moved Permanently";
		case 302: return "Found";
		case 303: return "See Other";
		case 304: return "Not Modified";
		case 307: return "Temporary Redirect";
		case 308: return "Permanent Redirect";
		case 400: return "Bad Request";
		case 401: return "Unauthorized";
		case 403: return "Forbidden";
		case 404: return "Not Found";
		case 405: return "Not Allowed";
		case 408: return "Request Timeout";
		case 409: return "Conflict";
		case 410: return "Gone";
		case 413: return "Payload Too Large";
		case 414: return "URI Too Long";
		case 415: return "Unsupported Media Type";
		case 429: return "Too Many Requests";
		case 500: return "Internal Server Error";
		case 501: return "Not Implemented";
		case 502: return "Bad Gateway";
		case 503: return "Service Unavailable";
		case 504: return "Gateway Timeout";
		case 505: return "HTTP Version Not Supported";
		default: return "Unknown Error";
	}
}

std::string Utils::getMimeType(std::string const &path) {
	static std::map<std::string, std::string> mimeTypes;
	if (mimeTypes.empty()) {
		mimeTypes[".avi"] = "video/x-msvideo";
		mimeTypes[".css"] = "text/css";
		mimeTypes[".csv"] = "text/csv";
		mimeTypes[".eot"] = "application/vnd.ms-fontobject";
		mimeTypes[".gif"] = "image/gif";
		mimeTypes[".gz"] = "application/gzip";
		mimeTypes[".htm"] = "text/html";
		mimeTypes[".html"] = "text/html";
		mimeTypes[".ico"] = "image/x-icon";
		mimeTypes[".ico"] = "image/x-icon";
		mimeTypes[".jpeg"] = "image/jpeg";
		mimeTypes[".jpg"] = "image/jpeg";
		mimeTypes[".js"] = "application/javascript";
		mimeTypes[".json"] = "application/json";
		mimeTypes[".mkv"] = "video/x-matroska";
		mimeTypes[".mp3"] = "audio/mpeg";
		mimeTypes[".mp4"] = "video/mp4";
		mimeTypes[".mpeg"] = "video/mpeg";
		mimeTypes[".ogg"] = "video/ogg";
		mimeTypes[".otf"] = "font/otf";
		mimeTypes[".pdf"] = "application/pdf";
		mimeTypes[".png"] = "image/png";
		mimeTypes[".svg"] = "image/svg+xml";
		mimeTypes[".tar"] = "application/x-tar";
		mimeTypes[".ttf"] = "font/ttf";
		mimeTypes[".txt"] = "text/plain";
		mimeTypes[".webm"] = "video/webm";
		mimeTypes[".webmanifest"] = "application/manifest+json";
		mimeTypes[".webp"] = "image/webp";
		mimeTypes[".woff"] = "font/woff";
		mimeTypes[".woff2"] = "font/woff2";
		mimeTypes[".xhtml"] = "application/xhtml+xml";
		mimeTypes[".xml"] = "application/xml";
		mimeTypes[".zip"] = "application/zip";
	}
	std::string::size_type i = path.rfind('.');
	if (i != std::string::npos) {
		std::string type = path.substr(i);
		if (mimeTypes.find(type) != mimeTypes.end()) {
			return mimeTypes[type];
		}
	}
	return "application/octet-stream";
}

std::string Utils::buildPage(std::vector<std::string> files, std::string path, std::string root) {
	std::sort(files.begin(), files.end());
	std::string page;
	std::string header = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Directory list</title><link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\"></head>";
	std::string body = "<body><div class=\"container\"><div class=\"display-6\">Index of " + path.substr(root.size()) + "</div><br><ul>";
	for (std::vector<std::string>::iterator it = files.begin(); it != files.end(); ++it) {
		body += "<li><a class=\"h5 btn btn-light rounded-pill p-0 px-3\" href=\"";
		body += *it;
		body += "\">";
		body += *it;
		body += "</a></li>";
	}
	body += "</ul></div></body></html>";
	return header + body;
}

void Utils::cleanPath(std::string& path) {
	if (path[0] != '/') {
        path.insert(0, "/");
	}
    if (path[path.size() - 1] != '/') {
        path += "/";
	}
    size_t pos;
    while ((pos = path.find("/./")) != std::string::npos) {
        path.erase(pos, 2);
    }
	while ((pos = path.find("/../")) != std::string::npos) {
		if (pos == 0) {
			path.erase(0, 3);
			continue;
		}
		size_t prev = path.rfind('/', pos - 1);
		if (prev != std::string::npos) {
			path.erase(prev, pos - prev + 3);
		} else {
			path.erase(0, pos + 3);
		}
	}
}

bool Utils::isPathWithinRoot(std::string const &root, std::string &path) {
	size_t i = -1;
	while (++i < root.size() && i < path.size()) {
		if (root[i] != path[i]) {
			break;
		}
	}
	return i == root.size();
}

std::string Utils::listDirectory(std::string path, std::string root) {
	Utils::cleanPath(path);
	if (path[0] != '.') {
		path.insert(0, ".");
	}
	Logger::log(Logger::DEBUG, "Root: %s", root.c_str());
	Logger::log(Logger::DEBUG, "Path: %s", path.c_str());
	if (!Utils::isPathWithinRoot(root, path)) {
		Logger::log(Logger::ERROR, "Path asked is not within root");
		return ErrorPage::getPage(403);
	}
	DIR *dir = opendir(path.c_str());
	if (!dir) {
		Logger::log(Logger::ERROR, "Failed to open directory: %s", path.c_str());
		return ErrorPage::getPage(404);
	}
	std::vector<std::string> files;
	struct dirent *ent;
	while ((ent = readdir(dir))) {
		files.push_back(ent->d_name);
	}
	closedir(dir);
	std::string body = Utils::buildPage(files, path, root);
	std::string header = "HTTP/1.1 200 OK\r\n";
	header += "Content-Type: text/html\r\n";
	header += "Content-Length: " + Utils::intToString(body.size()) + "\r\n";
	header += "\r\n";
	return header + body;
}

std::string Utils::uint64ToString(u_int64_t num) {
    std::ostringstream oss;
    oss << num;
    return oss.str();
}

std::string Utils::toLowerCase(std::string const &str) {
    std::string res = str;
    std::transform(res.begin(), res.end(), res.begin(), ::tolower);
    return res;
}

// EXCEPTIONS

int IntException::code() const { return _code; }
