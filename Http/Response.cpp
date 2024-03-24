/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 23:08:48 by abizyane          #+#    #+#             */
/*   Updated: 2024/03/24 22:27:20 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

std::map<std::string, std::string> Response::_mimeMap;
std::map<e_statusCode, std::string> Response::_statusMap;

Response::Response(IRequest& request, ProcessRequest& parse, int port, Selector& selector): _request(&request), 
	_parse(&parse), _good(false), _state(RESPONSE), _selector(selector) {
	_bodyIndex = 0;
	_responsefileName = "/tmp/.ResponseBody";
	_responsefileName += to_str(rand() % 100000);
	_status = _parse->getStatusCode();
	_waitForCgi = false;
	Response::initMaps();
	if (_request != NULL) {
		_server = MainConf::getConf()->getServerByHostPort(port, _request->getHeaders()["Host"]);
		if (_server != NULL)
			if (_request->getUri().find("?") != std::string::npos){
				_query = _request->getUri().substr(_request->getUri().find("?") + 1);
				_request->setUri(_request->getUri().substr(0, _request->getUri().find("?")));
			}
			_location = _server->getUri(_request->getUri());
	}
	else{
		_server = MainConf::getConf()->getServerByHostPort(port, "");
		_location = NULL;
	}
	_prepareResponse();
}

bool    Response::good(){
	return _good;
}

int    Response::sent() {
	if (_request && _state == DONE && _request->getHeaders()["Connection"] == "keep-alive")
		return 2;
	else if (_state == DONE)
		return 1;
	return 0;
}

Response::ResponseException::ResponseException(e_statusCode status): __status(status){
}

const char* Response::ResponseException::what() const throw(){
	return _statusMap[__status].c_str();
}

e_statusCode    Response::ResponseException::getStatus( void ){
	return __status;
}

void	Response::_buildResponse(){
	_response += "HTTP/1.1 ";
	_response += _statusMap[_status] + "\r\n";
	
	if (_status >= 400){
		if (_file.is_open())
			_file.close();
		_headers.clear();
		std::srand(time(0));
		_openFile(_responsefileName, 1);
		std::string errPage;
		if (_location)
			errPage = _location->getErrPage(static_cast<int>(_status), DefaultPages::getPage(_status));
		else
			errPage = _server->getErrPage(static_cast<int>(_status), DefaultPages::getPage(_status));		
		_file.write(errPage.c_str(), errPage.size());
		_headers["Content-Length"] = to_str(errPage.size());
		_headers["Content-Type"] = "text/html";
		_file.seekg(0, std::ios::beg);
	}
	_headers["Server"] = "Nginx++/1.0.0 (Unix)";
	char dt[30];
	time_t tm = time(0);
	strftime(dt, 30, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&tm));
	_headers["Date"] = std::string(dt);
	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
		_response += it->first + ": " + it->second + "\r\n";
	_response += "\r\n";
	_good = true;
	_parse->setGood(true);
}

void	Response::_readFile(std::string resource){
	struct stat fileStat;
	stat(resource.c_str(), &fileStat);
	if (S_ISDIR(fileStat.st_mode)){
		_headers["Content-Type"] = "Dir";
		return;
	}
	_openFile(resource, 0);
	size_t size = static_cast<size_t>(fileStat.st_size);
	_headers["Content-Length"] = to_str(size);
	std::string ext = getExtension(resource);
	if (ext != "" && _mimeMap[ext] != "")
		_headers["Content-Type"] = _mimeMap[ext];
	else
		_headers["Content-Type"] = "octet-stream";
	char dt[30];
	strftime(dt, 30, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&fileStat.st_mtime));
	_headers["Last-Modified"] = std::string(dt);
}

void	Response::_processGetResponse(){
	if (!_location->methodIsAllowed(_request->getMethod()))
		throw Response::ResponseException(HTTP_METHOD_NOT_ALLOWED);
	std::string resource = normPath(_location->getRoot() + normPath(_request->getUri()));
	_readFile(resource);
	if (_headers["Content-Type"] == "Dir"){
		_headers.erase("Content-Type");
		if (*(_request->getUri().end() - 1) != '/') {
			_headers["Location"] = _request->getUri() + "/";
			throw Response::ResponseException(HTTP_MOVED_PERMANENTLY);
		}
		if (_location->hasIndex()){
			std::vector<std::string>	indexes = _location->getIndex();
			for (size_t i = 0; i < indexes.size(); i++){
				std::string	tmp = _location->getRoot() + "/" + normPath(indexes[i]);
				if (!access(tmp.c_str(), F_OK)){
					// _readFile(tmp);
					_request->setUri(tmp);
					goto HERE;
				}
			}
		}
		if (!_location->dirListingEnabled())
			throw Response::ResponseException(HTTP_FORBIDDEN);
		if (*(resource.end() - 1) == '/')
			resource.erase(resource.size() - 1);
		resource = _autoIndex(normPath(resource));
		_headers.clear();
		_readFile(resource);
		_responsefileName = resource;
		return;
	}
	HERE:
	_handleRange();
	if (_location->hasCgi() && resource.find_last_of('.') != std::string::npos && _location->isCgi(resource.substr(resource.find_last_of('.')))){
		_waitForCgi = true;
		_headers.clear();
		_request->setUri(_request->getUri() + "?" + _query);
		_setCGI_Arguments();
		_initCGI();
		_executeCGI(_parse->getCgiFd());
	}
}

void	Response::_getFileName(std::string &resource) {
	std::time_t currentTime = std::time(0);
    char timestamp[100];
	std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d%H:%M:%S", std::localtime(&currentTime));
	std::string	extension = getExtension(_request->getUri());
	if (extension == "" || _mimeMap[extension] == "" ){
		extension = _request->getHeaders()["Content-Type"];
		if (extension != "")
			for (std::map<std::string, std::string>::iterator it = _mimeMap.begin(); it != _mimeMap.end(); it++)
				if (it->second == extension){
					extension = it->first;
					break;
				}
	}
	resource += "/" + std::string(timestamp);
	if (extension != "")
		resource += "." + extension;
	resource = normPath(resource);
	if (extension != "" && _mimeMap[extension] == "") // TODO: this is not working very well
		throw Response::ResponseException(HTTP_UNSUPPORTED_MEDIA_TYPE);
	return;
}

void	Response::_writeFile(std::string resource){
	struct stat st;
	std::string fileName;
	if (_location->hasUpload()){
		if (resource != "" && stat(resource.c_str(), &st) == -1)
				mkdir(resource.c_str(), 0777);
		_getFileName(resource);
		_openFile(resource, 1);
		_file.write(_request->getBody().data(), _request->getBody().size());
		_file.close();
		_headers["Content-Length"] = to_str(_request->getBody().size());
		if (_request->getHeaders()["Content-Type"] != "")
			_headers["Content-Type"] = _request->getHeaders()["Content-Type"];
		else
			_headers["Content-Type"] = _mimeMap[getExtension(resource)];
		if (_headers["Content-Type"] == "")
			_headers["Content-Type"] = "application/octet-stream";
		char dt[30];
		strftime(dt, 30, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&st.st_mtime));
		_headers["Last-Modified"] = std::string(dt);
		_status = HTTP_CREATED;
		return ;
	}
	_openFile(resource, 0);
	stat(resource.c_str(), &st);
	if (S_ISDIR(st.st_mode))
		_headers["Content-Type"] = "Dir";
}


void	Response::_processPostResponse(){
	if (!_location->methodIsAllowed(_request->getMethod()))
		throw Response::ResponseException(HTTP_METHOD_NOT_ALLOWED);
	std::string resource = _location->getRoot() + normPath(_location->getUploadStore()) + normPath(_request->getUri());
	_writeFile(resource);
	if (_status != HTTP_CREATED){
		if (_headers["Content-Type"] == "Dir"){
			_headers.erase("Content-Type");
			if (*(_request->getUri().end() - 1) != '/'){
				_headers["Location"] = _request->getUri() + "/";
				throw Response::ResponseException(HTTP_MOVED_PERMANENTLY);
			}
			if (!_location->hasIndex())
				throw Response::ResponseException(HTTP_FORBIDDEN);
			std::vector<std::string>	indexes = _location->getIndex();
			for (size_t i = 0; i < indexes.size(); i++){
				std::string	tmp = normPath(_location->getRoot() + "/" + normPath(indexes[i]));
				if (!access(tmp.c_str(), F_OK)){
					// _readFile(tmp);
					_request->setUri(tmp);
					break;
				}
			}
		}
	}
	else {
		_openFile(_responsefileName, 1);
		_file.write("File uploaded successfully", 24);
		_file.close();
		_headers["Content-Length"] = "24";
		_headers["Content-Type"] = "text/plain";
		_openFile(_responsefileName, 0);
	}
	if (_location->hasCgi() && resource.find_last_of('.') != std::string::npos && _location->isCgi(resource.substr(resource.find_last_of('.')))){
		_waitForCgi = true;
		_headers.clear();
		_request->setUri(_request->getUri() + "?" + _query);
		_setCGI_Arguments();
		_initCGI();
		_executeCGI(_parse->getCgiFd());
	}
}

void	Response::_deleteFile(std::string resource){
	struct stat st;
	if (stat(resource.c_str(), &st) == -1)
		throw Response::ResponseException(HTTP_NOT_FOUND);

	if (remove(resource.c_str()) != 0)
		throw Response::ResponseException(HTTP_INTERNAL_SERVER_ERROR);
	else if (!(st.st_mode & S_IRWXU))
		throw Response::ResponseException(HTTP_FORBIDDEN);
	_status = HTTP_NO_CONTENT;
}

void	Response::_processDeleteResponse(){
	_processGetResponse();
	_status = HTTP_NO_CONTENT;
}

void	Response::_handleRange(){
	try {
		if (_request->getHeaders()["Range"] != "") {
			std::string range = _request->getHeaders()["Range"];
			ssize_t length = strtoll(_headers["Content-Length"].c_str(), NULL, 10);
			ssize_t start = strtoll(range.substr(range.find("=") + 1, range.find("-")).c_str(), NULL, 10);
			range.erase(0, range.find("-") + 1);
			ssize_t end = length - 1;
			if (range != "")
				end = strtoll(range.c_str(), NULL, 10);
			_file.seekg(start, std::ios::beg);
			if (end > length || start > length || start > end || start < 0 || end < 0)
				throw Response::ResponseException(HTTP_REQUESTED_RANGE_NOT_SATISFIABLE);
			_headers["Content-Range"] = "bytes " + to_str(start) + "-" + to_str(end) + "/" + _headers["Content-Length"];
			_headers["Content-Length"] = to_str(end - start + 1);
			_status = HTTP_PARTIAL_CONTENT;
		}
	}
	catch (std::exception &){
		throw Response::ResponseException(HTTP_REQUESTED_RANGE_NOT_SATISFIABLE);
	}
}

void	Response::_handleCookies(){
	std::string	cookie = _request->getHeaders()["Cookie"];
	std::map<std::string, std::string>	_cookies;
	
	if (cookie != ""){
		std::vector<std::string>	cookies = _splitHeaderValue("Cookie");
		for (size_t i = 0; i < cookies.size(); i++){
			std::string		key = cookies[i].substr(0, cookies[i].find("="));
			std::string		value = cookies[i].substr(cookies[i].find("=") + 1);
			_cookies[key] = value;
		}
	}

// if (!cookieHeader.empty()) {
//     std::vector<std::string> cookiePairs = split(cookieHeader, ';');
//     for (const auto& pair : cookiePairs) {
//         std::vector<std::string> nameValue = split(pair, '=');
//         if (nameValue.size() == 2) {
//             cookies[nameValue[0]] = nameValue[1];
//         }
//     }
// }

}

std::string    Response::GetResponse(size_t lastSent){
	std::string		response;
	ssize_t index;
	switch (_state){
		case RESPONSE:
			response = _response.substr(0, _response.find("\r\n\r\n") + 4);
			_state = BODY;
			_bodyIndex -= response.size();
			break;
		case BODY:
			index = strtoll(_headers["Content-Length"].c_str(), NULL, 10);
			_response.erase(0, lastSent);
			_bodyIndex += lastSent;
			if (_response.size() == 0 && _bodyIndex < index && !_file.eof()){
				std::vector<char> buffer(index - _bodyIndex);
				_file.read(buffer.data(), index - _bodyIndex);
				_selector.unset(_parse->getCgiFd(), Selector::RD_SET);
				_parse->getCgiFd() = -1;
				std::streamsize readed = _file.gcount();
				_response.assign(buffer.data(), readed);
			}
			if (_response.size() > 0 && _bodyIndex < index)
				response = _response.substr(0, index - _bodyIndex);
			else{
				_state = DONE;
				std::remove(_responsefileName.c_str());
			}
			break;
		default:
			break;
    }
	return response;
}

void    Response::_prepareResponse(){
	try{
		if (_status == HTTP_OK){
			if (!_location)
				throw Response::ResponseException(HTTP_NOT_FOUND);
			if (_request->getBodySize() / 1048576.0f > _location->getClientBodySize())
				throw Response::ResponseException(HTTP_REQUEST_ENTITY_TOO_LARGE);
			else if (_location->hasRedirect()){
				_headers["Location"] = _location->getRedirectPage().second;
				throw Response::ResponseException(static_cast<e_statusCode>(_location->getRedirectPage().first));	
			}
			std::string  methods[3] = {"GET", "POST", "DELETE"};
			for (int i = 0; i < 3; i++)
				if (_request->getMethod() == methods[i])
					switch (i){
						case 0:
							_processGetResponse();
							break;
						case 1:
							_processPostResponse();
							break;
						case 2:
							_processDeleteResponse();
						default:
							break;
					}
		}
	}
	catch (Response::ResponseException &e){
		_status = e.getStatus();
	}
	TRYAGAIN:
	try {
		if (!_waitForCgi)
			_buildResponse();
	}
	catch (Response::ResponseException &e){
		_status = e.getStatus();
		_response.clear();
		goto TRYAGAIN;
	}
}

Response::~Response(){
	if (_file.is_open())
		_file.close();
}

std::string	Response::_autoIndex( const std::string& dirName ){
	std::string	htmlPage;

	htmlPage += "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
	htmlPage += "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css\" integrity=\"sha512-DTOQO9RWCH3ppGqcWaEA1BIZOC6xxalwEsw9c2QQeAIftl+Vegovlnee1c9QX4TctnWMn13TZye+giMm8e2LwA==\" crossorigin=\"anonymous\" referrerpolicy=\"no-referrer\" />\n";
	htmlPage += "<title>read" + dirName + "</title><style> .fa-file { color: #85c3ec; padding-right: 15px;}a {text-decoration: none; color: #094f7e;} ";
	htmlPage += "i { color: #bea307; padding-right: 10px; } table { border-collapse: collapse; } th, td { padding: 10px 20px ; text-align: left; } ";
	htmlPage += "th { background-color: #ffffff; }		\n";
	htmlPage += "tr:nth-child(even) { background-color: #f8f7f7; } tr:hover { background-color: #ddd; }	td {cursor: pointer;} </style> </head> ";
	DIR*		dir;
	if ((dir = opendir(dirName.c_str())) != NULL) {
		htmlPage += "<body><h1>Index of [ " + dirName + " ]: </h1><hr><h2 style=\"padding: 10px;\"><a href=\"../\"><i class=\"fa-solid fa-backward\"></i> parent dir ... </a> </h2><table><tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>";

		// htmlPage += "<body><h1>Index of [ " + dirName + " ]: </h1><hr><h2 style=\"padding: 10px;\"><i class=\"fa-solid fa-folder\"></i> parent dir: <a href=\"" + dirName + "/..\"> ..... </a> </h2><table><tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>";
		struct dirent*	ent;
		while ((ent = readdir(dir)) != NULL) {
			std::string	fileName = ent->d_name;
			if (fileName == "." || fileName == "..")
				continue;
			std::string	filePath = dirName + "/" + fileName;
			struct stat statbuff;
			if (stat(filePath.c_str(), &statbuff) != -1) {
				char buffer[30];
				strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&statbuff.st_mtime));
				std::string	lastModified = buffer;
				std::string	fileSize = to_str(statbuff.st_size);
				if (S_ISDIR(statbuff.st_mode)) {
					htmlPage += "<tr><td><a href=\"" + fileName + " \"><i class=\"fa-solid fa-folder\"></i> " + fileName + "</a></td>";
				} else {
					htmlPage += "<tr><td><a href=\"" + fileName + " \"><i class=\"fa-solid fa-file\"></i> " + fileName + "</a></td>";
				}
				htmlPage += "<td>" + lastModified + "</td><td>" + fileSize + "</td></tr>";
			}
		}
		closedir(dir);
	} else {
		htmlPage += "<h1>Error couldn't opreaden the directory : " + dirName  + "</h1>";
	}
	htmlPage += "</table></body></html>";
	_responsefileName += ".html";
	_openFile(_responsefileName, 1);
	_file << htmlPage;
	_file.close();
	return _responsefileName;
}

//  ========== For CGI ==========

void    Response::_setCGI_Arguments( void ) {
    std::string requestURI = _request->getUri();
    std::string serverRoot = _location->getRoot();
    _file_path = serverRoot + requestURI;
    _query_string = "";
    _cgi_argv = new char*[2];
    if (requestURI.find('?') != std::string::npos) {
        _query_string = requestURI.substr(requestURI.find('?') + 1);
        _file_path = serverRoot + requestURI.substr(0, requestURI.find('?'));
    }
    _cgi_argv[0] = strdup(normPath(_file_path).c_str());
    _cgi_argv[1] = NULL;
}

void    Response::_initCGI() {
    std::string requestUri = _request->getUri();
    std::string locationRoot = _location->getRoot();
    std::map<std::string, std::string> headers = _request->getHeaders();

    headers["SERVER_PROTOCOL"] = "HTTP/1.1";
	headers["GATEWAY_INTERFACE"] = "CGI/1.1";
	headers["SERVER_SOFTWARE"] = "webserv/1.0";
    headers["REDIRECT_STATUS" ] = "0";
    headers["HTTP_COOKIE"] = headers["Cookie"];
    headers["HTTP_USER_AGENT"] = headers["User-Agent"];
    headers["HTTP_HOST"] = headers["Host"];
    headers["SERVER_PORT"] = to_str(_parse->getPort());
    headers["PATH_INFO"] = _file_path;
    headers["PATH_TRANSLATED"] = _file_path;
    headers["SCRIPT_NAME"] = _file_path;
    headers["REQUEST_URI"] = _file_path;
    headers["DOCUMENT_ROOT"] = locationRoot;
    headers["HTTP_CONNECTION"] = headers["Connection"];
    headers["HTTP_ACCEPT"] = headers["Accept"];
    headers["HTTP_USER-AGENT"] = headers["User-Agent"];
    headers["HTTP_COOKIE"] = headers["Cookie"];
    headers["CONTENT_TYPE"] = headers["Content-Type"];
    headers["REQUEST_METHOD"] = _request->getMethod();
    headers["QUERY_STRING"] = _query_string;
    // ---------------
    std::map<std::string, std::string>::iterator it = headers.begin();
    while (it != headers.end())
    {
        std::string key = it->first;
        if (std::islower(key[1]))
            key = formKey(key);
        setenv(key.c_str(), it->second.c_str(), 1);
        it++;
    }
}

int    Response::_executeCGI( int& fd ) {
    fd = open(_responsefileName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1) {
		_status = HTTP_INTERNAL_SERVER_ERROR;
		return 1;
	}
	_cgi_pid = fork();
	if (_cgi_pid == -1) {
		_status = HTTP_INTERNAL_SERVER_ERROR;
		return 1;
	}
    if (_cgi_pid == 0) {
        dup2(fd, 1);
		close(fd);
        int fd1 = open(_request->getFileName().c_str(), O_RDONLY, 0666);
		if (fd1 > 0) {
	        dup2(fd1, 0);
        	close(fd1);
		}
        extern char** environ;
		
        execve(_cgi_argv[0], _cgi_argv, environ);
		std::cerr << _cgi_argv[0] << " " <<  strerror(errno) << std::endl;
		exit(502);
    }
    fcntl(fd, F_SETFL, O_NONBLOCK);
    _selector.set(fd, Selector::RD_SET);
	// wait(NULL);
	// _getCGI_Response();
	return 0;
}

static std::string trim(std::string &s) {
	int index_begin = 0;
	int index_end = s.size() - 1;
	int found = 0;
	while (index_begin < index_end && found < 2) {
		if (isspace(s[index_begin]) || s[index_begin] == '\n') {
    	    index_begin++;
    	} else if (isspace(s[index_end]) || s[index_end] == '\n') {
    	    index_end--;
    	} else {
    	    break;
    	}
	}
	s = s.substr(index_begin, index_end - index_begin + 1);
	return s;
}

void splitString(const std::string& str, std::vector<std::string>& container, char delimiter = '\r') {
    std::istringstream iss(str);
    std::string item;

    while (std::getline(iss, item, delimiter)) {
        container.push_back(trim(item));
    }
}

void 		Response::_parseCgiHeaders( std::string headers) {
	size_t position = headers.find("\r\n");
	std::string responseLine = headers.substr(0, position);
	_status = static_cast<e_statusCode> (atoi((responseLine.substr(responseLine.find(" "), responseLine.find_last_of(' ')).c_str())));
	headers.erase(0, position + 1);
	trim(headers);
	std::vector<std::string> headers_lines;
	splitString(headers, headers_lines);
	std::vector<std::string>::iterator it = headers_lines.begin();
	while (it != headers_lines.end()) {
		std::string key = it->substr(0, it->find_first_of(':'));
		std::string value = it->substr(it->find(": ") + 2);
		_headers[key] = value;
		it++;
	}
}

int Response::_getCGI_Response(void) {
    std::string headers;
    std::string body;
    int status;
    int ret = waitpid(_cgi_pid, &status, WNOHANG);
    if (ret == _cgi_pid) {
        if ((WIFEXITED(status) && WEXITSTATUS(status) != 0) || WIFSIGNALED(status)) {
            _status = HTTP_INTERNAL_SERVER_ERROR;
            goto Here;
        }
        std::ifstream cgiResponse(_responsefileName);
        if (!cgiResponse.is_open()) {
            _status = HTTP_FORBIDDEN;
            goto Here;
        }
        try {
            std::stringstream buffer;
            buffer << cgiResponse.rdbuf();
            cgiResponse.close();
            std::string allfile = buffer.str();

            size_t pos = allfile.find("\r\n\r\n");
            if (pos != std::string::npos) {
                headers = allfile.substr(0, pos);
                body = allfile.substr(pos + 4);
                _parseCgiHeaders(headers);
				_file.close();
				std::remove(_responsefileName.c_str());
				_file.open(_responsefileName.c_str(), std::ios::in | std::ios::out | std::ios::trunc);
                if (!_file.is_open()) {
                    _status = HTTP_INTERNAL_SERVER_ERROR;
                    goto Here;
                }
				_file.write(body.c_str(), body.size());
				_file.close();
				_file.open(_responsefileName.c_str(), std::ios::in | std::ios::out);
            }
        } catch (const std::exception &) {
            _status = HTTP_INTERNAL_SERVER_ERROR;
            goto Here;
        }
    } else {
        return 1;
    }
	Here:
	_buildResponse();
    return 0;
}