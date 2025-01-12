#include "CgiExecutor.hpp"

CgiExecutor::CgiExecutor(RequestCgi *requestCgi) : _requestCgi(requestCgi), _envp(NULL), _argv(NULL), _body(true), _pid(FAIL), _tmpStdin(FAIL), _tmpStdout(FAIL) {}

CgiExecutor::CgiExecutor(const CgiExecutor &obj) { *this = obj; }

CgiExecutor::~CgiExecutor() {
	if (_envp) {
		for (int i = 0; _envp[i]; i++)
			delete[] _envp[i];
		delete[] _envp;
	}
	if (_argv) {
		for (int i = 0; _argv[i]; i++) {
			delete[] _argv[i];
		}
		delete[] _argv;
	}
	if (_tmpStdin != FAIL)	{
		close(_tmpStdin);
		_tmpStdin = FAIL;
	}
	if (_tmpStdout != FAIL) {
		close(_tmpStdout);
		_tmpStdout = FAIL;
	}
}

CgiExecutor &CgiExecutor::operator=(const CgiExecutor &obj) {
	if (this != &obj) {
		_requestCgi = obj._requestCgi;
	}
	return *this;
}

void CgiExecutor::init() {
	Logger::log(Logger::DEBUG, "[CgiExecutor::init] Start CGI Executor");
	Logger::log(Logger::DEBUG, "[CgiExecutor::init] Path: %s", _requestCgi->_path.c_str());
	Logger::log(Logger::DEBUG, "[CgiExecutor::init] ExecPath: %s", _requestCgi->_execPath.c_str());

	_env["SERVER_SOFTWARE"] = "webserv/1.0";
	_env["SERVER_NAME"] = _requestCgi->_request->_headers["Host"];
	_env["SERVER_PROTOCOL"] = _requestCgi->_request->_httpVersion;
	_env["SERVER_PORT"] = Utils::intToString(_requestCgi->_request->_client->getSocket()->getPort());
	_env["REDIRECT_STATUS"] = "200";
	_env["REQUEST_METHOD"] = _requestCgi->_request->_httpMethod;
	_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	_env["SCRIPT_NAME"] = _requestCgi->_path;
	_env["SCRIPT_FILENAME"] = _requestCgi->_path;
	_env["PATH_INFO"] = _requestCgi->_path;
	_env["PATH_TRANSLATED"] = _requestCgi->_path; // Not implemented
	_env["QUERY_STRING"] = _requestCgi->_request->_query;
	_env["REQUEST_URI"] = _requestCgi->_request->_uri;
	_env["REMOTE_ADDR"] = _requestCgi->_request->_client->getSocket()->getIp();
	_env["REMOTE_IDENT"] = _requestCgi->_request->_headers["Authorization"];
	_env["REMOTE_USER"] = _requestCgi->_request->_headers["Authorization"];
	_env["CONTENT_LENGTH"] = Utils::ullToStr(_requestCgi->_request->_body._size);
	_env["CONTENT_TYPE"] = _requestCgi->_request->_headers["Content-Type"];
	_env["HTTP_COOKIE"] = _requestCgi->_request->_headers["Cookie"];
}

void CgiExecutor::execute() {
	Logger::log(Logger::DEBUG, "[CgiExecutor::execute] Start CGI Executor");
	_envp = envToChar();
	_argv = buildArgv();
	_tmpStdin = dup(STDIN_FILENO);
	_tmpStdout = dup(STDOUT_FILENO);

	if (Utils::createTempFile(_body._path, _body._fd) == FAIL) {
		throw std::invalid_argument("[CgiExecutor::execute] createTempFile failed");
	}
	if (_requestCgi->_request->_body._fd != FAIL) {
		if (lseek(_requestCgi->_request->_body._fd, 0, SEEK_SET) == FAIL) {
			throw std::invalid_argument("[CgiExecutor::execute] lseek failed");
		}
	}
	_pid = fork();
	if (_pid == FAIL) {
		throw std::invalid_argument("[CgiExecutor::execute] fork failed");
	}
	if (_pid == 0) {
		if (_requestCgi->_request->_body._fd != FAIL) {
			if (dup2(_requestCgi->_request->_body._fd, STDIN_FILENO) == FAIL) {
				throw ChildProcessException();
			}
		}
		if (dup2(_body._fd, STDOUT_FILENO) == FAIL) {
			throw ChildProcessException();
		}
		execve(_argv[0], _argv, _envp);
		throw ChildProcessException();
	} else {
		_requestCgi->_request->setState(Request::CGI_PROCESS);
	}
}

char **CgiExecutor::envToChar() {
	char **envp = new char*[_env.size() + 1];
	std::map<std::string, std::string>::const_iterator it;
	int i = FAIL;

	for (it = _env.begin(); it != _env.end(); it++) {
		std::string env = it->first + "=" + it->second;
		envp[++i] = new char[env.size() + 1];
		strcpy(envp[i], env.c_str());
	}
	envp[++i] = NULL;
	return envp;
}

char **CgiExecutor::buildArgv() {
	char	**argv = new char*[3];
	argv[0] = new char[_requestCgi->_execPath.size() + 1];
	strcpy(argv[0], _requestCgi->_execPath.c_str());
	argv[1] = new char[_requestCgi->_path.size() + 1];
	strcpy(argv[1], _requestCgi->_path.c_str());
	argv[2] = NULL;
	return argv;
}

int	CgiExecutor::getFdOut() const { return _body._fd; }
