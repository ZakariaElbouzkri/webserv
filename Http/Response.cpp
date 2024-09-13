/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 23:08:48 by abizyane          #+#    #+#             */
/*   Updated: 2024/09/13 14:58:47 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

std::map<std::string, std::string> Response::_mimeMap;
std::map<e_statusCode, std::string> Response::_statusMap;

Response::Response(IRequest& request, ProcessRequest& parse, int port, Selector& selector): _request(&request), 
	_parse(&parse), _good(false), _state(RESPONSE), _selector(selector) {
	_bodyIndex = 0;
	_responsefileName = "/tmp/.ResponseBody";
	std::srand(std::time(0));
	_responsefileName += to_str(rand() % 100000);
	_status = _parse->getStatusCode();
	_waitForCgi = false;
	Response::initMaps();
	if (_request != NULL) {
		_server = MainConf::getConf()->getServerByHostPort(port, _request->getHeaders()["Host"]);
		if (_server != NULL){
			if (_request->getUri().find("?") != std::string::npos){
				_query = _request->getUri().substr(_request->getUri().find("?") + 1);
				_request->setUri(_request->getUri().substr(0, _request->getUri().find("?")));
			}
			_request->setUri(decodeURI(_request->getUri()));
			std::pair<std::string, LocationConf*>	ans = _server->getUri(_request->getUri());
			_location = ans.second;
			if (!_location){
				_status = HTTP_NOT_FOUND;
				goto X;
			}
			_request->setUri(ans.first);
		}
	}
	else{
		_server = MainConf::getConf()->getServerByHostPort(port, "");
		_location = NULL;
	}
	X:
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
	
	if (_cookies.size() > 0 && _waitForCgi == false)
		for (size_t i = 0; i < _cookies.size(); i++){
			_response += "Set-Cookie: ";
			for (size_t j = 0; j < _cookies[i].size(); j++){
				if (_cookies[i][j].second == "")
					_response += _cookies[i][j].first + "; ";
				else
					_response += _cookies[i][j].first + "=" + _cookies[i][j].second + "; ";
			}
			_response.erase(_response.size() - 2);
			_response += "\r\n";
		}
	_response += "\r\n";
	_good = true;
	_parse->setGood(true);
}	

void	Response::_processGetResponse(){
	if (!_location->methodIsAllowed(_request->getMethod()))
		throw Response::ResponseException(HTTP_METHOD_NOT_ALLOWED);
	std::string resource = normPath(_location->getRoot() + normPath(_request->getUri()));
	if (_location->hasRedirect()){
		_headers["Location"] = _location->getRedirectPage().second;
		throw Response::ResponseException(static_cast<e_statusCode>(_location->getRedirectPage().first));
	}
	_readFile(resource);
	if (_headers["Content-Type"] == "Dir"){
		_headers.erase("Content-Type");
		if (_location->hasIndex()){
			std::vector<std::string>	indexes = _location->getIndex();
			for (size_t i = 0; i < indexes.size(); i++){
				std::string	tmp = _location->getRoot() + normPath(indexes[i]);
				if (!access(tmp.c_str(), F_OK)){
					_request->setUri(normPath(indexes[i]));
					resource = tmp;
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
	_handleCookies();
	if (_location->hasCgi() && resource.find_last_of('.') != std::string::npos && _location->isCgi(resource.substr(resource.find_last_of('.')))){
		_waitForCgi = true;
		_headers.clear();
		_request->setUri(_request->getUri() + "?" + _query);
		_setCGI_Arguments();
		_initCGI();
		_executeCGI(_parse->getCgiFd());
	}
}

void	Response::_processPostResponse(){
	if (!_location->methodIsAllowed(_request->getMethod()))
		throw Response::ResponseException(HTTP_METHOD_NOT_ALLOWED);
	std::string resource = normPath(_location->getRoot() + normPath(_request->getUri()));
	if (_location->hasRedirect()){
		_headers["Location"] = _location->getRedirectPage().second;
		throw Response::ResponseException(static_cast<e_statusCode>(_location->getRedirectPage().first));
	}
	struct stat st;
	if (stat(resource.c_str(), &st) == 0 && S_ISDIR(st.st_mode)){
		if (!_location->hasIndex())
			throw Response::ResponseException(HTTP_FORBIDDEN);
		std::vector<std::string>	indexes = _location->getIndex();
		for (size_t i = 0; i < indexes.size(); i++){
			std::string	tmp = _location->getRoot() + normPath(indexes[i]);
			if (!access(tmp.c_str(), F_OK)){
				_request->setUri(normPath(indexes[i]));
				resource = tmp;
				break;
			}
		}
	}
	_handleCookies();
	if (_location->hasCgi() && resource.find_last_of('.') != std::string::npos && _location->isCgi(resource.substr(resource.find_last_of('.')))){
		_waitForCgi = true;
		_headers.clear();
		_request->setUri(_request->getUri() + "?" + _query);
		_setCGI_Arguments();
		_initCGI();
		_executeCGI(_parse->getCgiFd());
	}
	else
		throw Response::ResponseException(HTTP_UNPROCESSABLE_ENTITY);
}

void	Response::_processPutResponse(){
	if (!_location->methodIsAllowed(_request->getMethod()))
		throw Response::ResponseException(HTTP_METHOD_NOT_ALLOWED);
	std::string resource = _location->getRoot() + normPath(_location->getUploadStore());
	struct stat st;
	if (stat(resource.c_str(), &st) == -1)
		mkdir(resource.c_str(), 0777);
	resource += normPath(_request->getUri());
	if (_location->hasRedirect()){
		_headers["Location"] = _location->getRedirectPage().second;
		throw Response::ResponseException(static_cast<e_statusCode>(_location->getRedirectPage().first));
	}
	if (_location->hasUpload()){
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
		_openFile(_responsefileName, 1);
		_file.write("File uploaded successfully", 24);
		_file.close();
		_headers["Content-Length"] = "24";
		_headers["Content-Type"] = "text/plain";
		_openFile(_responsefileName, 0);
	}
	else
		throw Response::ResponseException(HTTP_FORBIDDEN);
	_handleCookies();
}

void	Response::_processDeleteResponse(){
	if (!_location->methodIsAllowed(_request->getMethod()))
		throw Response::ResponseException(HTTP_METHOD_NOT_ALLOWED);
	std::string resource = normPath(_location->getRoot() + normPath(_request->getUri()));
	if (_location->hasRedirect()){
		_headers["Location"] = _location->getRedirectPage().second;
		throw Response::ResponseException(static_cast<e_statusCode>(_location->getRedirectPage().first));
	}
	_readFile(resource);
	_status = HTTP_NO_CONTENT;
	if (_headers["Content-Type"] == "Dir"){
		_headers.erase("Content-Type");
		if (*(_request->getUri().end() - 1) != '/')
			throw Response::ResponseException(HTTP_CONFLICT);
		if (_location->hasIndex()){
			std::vector<std::string>	indexes = _location->getIndex();
			for (size_t i = 0; i < indexes.size(); i++){
				std::string	tmp = _location->getRoot() + normPath(indexes[i]);
				if (!access(tmp.c_str(), F_OK)){
					_request->setUri(normPath(indexes[i]));
					resource = tmp;
					break;
				}
			}
		}
		else
			throw Response::ResponseException(HTTP_FORBIDDEN);
	}
	_handleCookies();
	if (_location->hasCgi() && resource.find_last_of('.') != std::string::npos && _location->isCgi(resource.substr(resource.find_last_of('.')))){
		_waitForCgi = true;
		_headers.clear();
		_request->setUri(_request->getUri() + "?" + _query);
		_setCGI_Arguments();
		_initCGI();
		_executeCGI(_parse->getCgiFd());
	}
	else
		throw Response::ResponseException(HTTP_UNPROCESSABLE_ENTITY);
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
			if (index == 0)
				goto X;
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
				X:
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
			std::string  methods[4] = {"GET", "POST", "DELETE", "PUT"};
			for (int i = 0; i < 4; i++)
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
							break;
						case 3:
							_processPutResponse();
						default:
							break;
					}
		}
	}
	catch (Response::ResponseException &e){
		_status = e.getStatus();
	}
	catch (std::exception &e){
		_status = HTTP_INTERNAL_SERVER_ERROR;
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