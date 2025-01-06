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
	std::string body = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>TITLE</title><style>@import url('https://fonts.googleapis.com/css2?family=Inter:ital,opsz,wght@0,14..32,100..900;1,14..32,100..900&display=swap');body{height: 100vh;margin: 0;padding: 0;display: flex;justify-content: center;align-items: center;background-color: #f1f1f1;font-family: \"Inter\", sans-serif;font-optical-sizing: auto;flex-direction: column;}.wrapper{position: relative;}.mainTitle{text-align: center;position: relative;font-size: 3rem;}.mainIcon{left: 50%;transform: translate(-50%, -100%);position: absolute;}.mainText{position: absolute;top: 50%;left: 50%;transform: translate(-50%, 120%);text-align: center;width: 100vw;}.subtitle{position: absolute;bottom: 0;}</style></head><body><div class=\"wrapper\"><lord-icon src=\"https://cdn.lordicon.com/usownftb.json\" trigger=\"loop\" delay=\"2000\" colors=\"primary:#000000,secondary:#000000\" style=\"width:150px;height:150px\" class=\"mainIcon\"></lord-icon><h1 class=\"mainTitle\">Error ERROR</h1><p class=\"mainText\">MESSAGE</p></div></body><script src=\"https://cdn.lordicon.com/lordicon.js\"></script></html>";
	body.replace(body.find("TITLE"), 5, Utils::intToString(statusCode));
	body.replace(body.find("ERROR"), 5, Utils::intToString(statusCode));
	body.replace(body.find("MESSAGE"), 7, message);
	std::string responseLine = "HTTP/1.1 " + Utils::intToString(statusCode) + " " + message + "\r\n";
	std::string headers = "Content-Type: text/html\r\nContent-Length: " + Utils::intToString(body.size()) + "\r\n\r\n";
	return responseLine + headers + body;
}
