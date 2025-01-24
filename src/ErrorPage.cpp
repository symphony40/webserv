#include "ErrorPage.hpp"

std::string ErrorPage::getErrorPagesCustom(int statusCode, std::map<int, std::string> errorPagesCustom){
	if (errorPagesCustom.empty()) {
		return "";
	}
	std::map<int, std::string>::iterator it = errorPagesCustom.find(statusCode);
	if (it == errorPagesCustom.end()) {
		return "";
	}	
	std::string path = it->second;
	std::string page;
	std::ifstream file(path.c_str());
	if (file.is_open())	{
		std::stringstream content;
		content << file.rdbuf();
		std::string body = content.str();
		page = "HTTP/1.1 " + Utils::intToString(statusCode) + " " + Utils::getHttpStatusMessage(statusCode) + "\r\n";
		page += "Content-Type: " + Utils::getMimeType(path) + "\r\n";
		page += "Content-Length: " + Utils::intToString(body.size()) + "\r\n";
		page += "\r\n";
		page += body;
		file.close();
		return page;
	} else {
		Logger::log(Logger::ERROR, "Failed to open custom Error Page: %s", path.c_str());
		return "";
	}
}

std::string ErrorPage::getPage(int statusCode, std::map<int, std::string> errorPagesCustom) {
	std::string customPage = getErrorPagesCustom(statusCode, errorPagesCustom);
	if (!customPage.empty()) {
		return customPage;
	}
	std::string message = Utils::getHttpStatusMessage(statusCode);
	std::string body = "<!DOCTYPE html><head><title>TITLE</title><link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css\" rel=\"stylesheet\"integrity=\"sha384-QWTKZyjpPEjISv5WaRU9OFeRpok6YctnYmDr5pNlyT2bRjXh0JMhjY6hW+ALEwIH\" crossorigin=\"anonymous\"></head><body><div class=\"d-flex align-items-center text-center justify-content-center bg-secondary vh-100\"><div><div><img alt=\"Warning SVG Vector Icon\" fetchpriority=\"high\" width=\"250\" height=\"250\" decoding=\"async\"data-nimg=\"1\" style=\"color:transparent;width:100px;height:100px\" src=\"warning.svg\"></div><div class=\"display-1\">ERROR</div><div class=\"display-6\">MESSAGE</div></div></div></body></html>";
	body.replace(body.find("TITLE"), 5, Utils::intToString(statusCode));
	body.replace(body.find("ERROR"), 5, Utils::intToString(statusCode));
	body.replace(body.find("MESSAGE"), 7, message);
	std::string responseLine = "HTTP/1.1 " + Utils::intToString(statusCode) + " " + message + "\r\n";
	std::string headers = "Content-Type: text/html\r\nContent-Length: " + Utils::intToString(body.size()) + "\r\n\r\n";
	return responseLine + headers + body;
}
