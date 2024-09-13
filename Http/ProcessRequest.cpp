/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessRequest.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/13 13:46:10 by abizyane          #+#    #+#             */
/*   Updated: 2024/02/13 16:38:49 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ProcessRequest.hpp"

ProcessRequest::ProcessRequest(int port, Selector& _selector, int &fd) :_port(port), _state(RequestLine), _status(HTTP_OK),
	_request(NULL), _response(NULL), _cgi_fd(fd), _good(false), _selector(_selector) {
}

IRequest*	ProcessRequest::getRequest( void ){
	return _request;
}

Response*	ProcessRequest::getResponse( void ){
	return _response;
}

bool	ProcessRequest::good(){
	return _good;
}

e_parseState	ProcessRequest::getParseState( void ){
	return _state;
}

void	ProcessRequest::setParseState(e_parseState state){
	_state = state;
}

e_statusCode	ProcessRequest::getStatusCode( void ){
	return _status;
}

void	ProcessRequest::setGood(bool good){
	_good = good;
}

static std::string	getToken(std::string &line) {
		std::string		token;
		size_t			index = line.find_first_of(" \t\n\r\f\v");

		token = line.substr(0, index);
		line.erase(0, index);
		line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
		return token;
}

static int		checkMethod(std::string& method){
	std::string str[11] = {"GET", "POST", "DELETE", "PUT", "HEAD",
		"CONNECT", "OPTIONS", "TRACE", "PATCH", "LINK", "UNLINK"};

	int i = 0;
	for (; i < 11; i++)
		if (str[i] == method)
			break;
	if (i < 4)
		return i;
	if (i < 11)
		return 501;
	return 400;
}

static int	checkUri(std::string& uri){
	// std::string allowedChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-._~:/?#[]@!$&'()*+,;=%0123456789");
	
	// if (uri[0] != '/' || uri.find_first_not_of(allowedChars) != std::string::npos)
	// 	return 400;
	if (uri.size() > 2048)
		return 414;
	return 200;
}

static int	checkVersion(std::string& version){
	if (version.compare(0, 5, "HTTP/"))
		return 400;
	return ((version == "HTTP/1.1")? 200 : 505);
}

void	ProcessRequest::parseLine(char *buffer, int size){
	std::string		line;

	_requestBuffer.append(buffer, size);
	while (_state != Body && (_requestBuffer.find("\r\n") != std::string::npos ||
			_requestBuffer.find("\n") != std::string::npos)){

		_requestBuffer.find("\r\n") != std::string::npos ?
			line = _requestBuffer.substr(0, _requestBuffer.find("\r\n")) :
				line = _requestBuffer.substr(0, _requestBuffer.find("\n"));

		if (_state == Headers && line.empty())
			_request->checkHeaders();

		switch (_state){
			case RequestLine:
				_parseRequestLine(line);
				break;
			case Headers:
				_status = _request->parseHeader(line);
				if (_status != HTTP_OK)
					_state = Error;
			default:
				break;
		}
		_requestBuffer.find("\r\n") != std::string::npos ?
			_requestBuffer.erase(0, _requestBuffer.find("\r\n") + 2) :
				_requestBuffer.erase(0, _requestBuffer.find("\n") + 1);
	}
	if (_state == Body)
		_status = _request->parseBody(_requestBuffer);

	if (_state == Error || _state == Done)
		_response = new Response(*_request, *this, _port, _selector);

}

void	ProcessRequest::_parseRequestLine(std::string &requestLine){
	std::string		method;
	std::string		uri;
	std::string		version;

	try {
		method = getToken(requestLine);	
		uri = getToken(requestLine);
		version = getToken(requestLine);

		_status = static_cast<e_statusCode>(checkMethod(method));
		switch (checkMethod(method)){
			case 0:
				_request = new GetRequest(method, uri, *this);		break;
			case 1:
				_request = new PostRequest(method, uri, *this);		break;
			case 2:
				_request = new DeleteRequest(method, uri, *this);	break;
			case 3:
				_request = new PutRequest(method, uri, *this);		break;
			default:
				_state = Error;										return;
		}
		if ((_status = static_cast<e_statusCode>(checkUri(uri))) != 200 ||
			(_status = static_cast<e_statusCode>(checkVersion(version))) != 200){
				_state = Error;
				return;
		}
	}catch(const std::out_of_range&){
		_status = HTTP_BAD_REQUEST;		
		_state = Error;
		return;
	}
	_state = Headers;
}

bool	ProcessRequest::sent( void ){
	int	i = _response->sent();
	switch (i){
		case 0:
			return false;
		case 1:
			return true;
		default:
			_resetProcessor();
	}	
	return false;
}

const int&		ProcessRequest::getPort() const {
	return _port;
}

void	ProcessRequest::_resetProcessor( void ){
	_state = RequestLine;
	_status = HTTP_OK;
	_requestBuffer.clear();
	_good = false;
	delete _request;
	_request = NULL;
	delete _response;
	_response = NULL;
}

int&			ProcessRequest::getCgiFd() {
	return _cgi_fd;
}

void			ProcessRequest::getCgiResponse() {
	_response->readCgiFile();
}

ProcessRequest::~ProcessRequest(){
	delete _request;
	delete _response;
}
