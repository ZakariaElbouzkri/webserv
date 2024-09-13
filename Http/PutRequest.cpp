/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PutRequest.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/25 16:14:26 by abizyane          #+#    #+#             */
/*   Updated: 2024/09/11 22:24:19 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PutRequest.hpp"

PutRequest::PutRequest(std::string &method, std::string &uri, ProcessRequest& parse)
    : _method(method), _uri(uri), _version("HTTP/1.1"), _parse(parse){
	_contentLength = 0;
	_bodyIndex = 0;
	_isChunked = false;
	_chunkLen = 0;
	_gotChunkLen = false;
	_fileName = "/tmp/.requestbody";
    std::srand(std::time(0));
    _fileName += to_str(std::rand() % 100000);
}

std::string		PutRequest::getMethod( void ) const{
	return _method;
}

std::string		PutRequest::getUri( void ) const{
	return _uri;
}

void	PutRequest::setUri( std::string uri ){
	_uri = uri;
}

std::map<std::string, std::string>	PutRequest::getHeaders( void ) const{
	return _headers;
}

std::vector<char>	PutRequest::getBody( void ) {
	struct stat	s;
	stat(_fileName.c_str(), &s);

	std::vector<char> buffer(s.st_size);
	_body.open(_fileName.c_str(), std::ios::in);
	_body.read(buffer.data(), s.st_size);
	_body.close();
	return buffer;
}

ssize_t	PutRequest::getBodySize( void ) const{
	struct stat	s;
	if (stat(_fileName.c_str(), &s) != 0)
		return 0;
	return s.st_size;
}

ProcessRequest&	PutRequest::getParse( void ) const{
	return _parse;
}

e_statusCode	PutRequest::parseHeader(std::string &line){
	try{
		if (line.find(":") == std::string::npos)
			return HTTP_BAD_REQUEST;
		std::string key = line.substr(0, line.find(":"));
		line.erase(0, line.find(":") + 1);
		line.erase(0, line.find_first_not_of(" \t\n\r\f\v"));
		line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);
		std::string value = line;
		line.clear();
        std::string allowedChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-!#$%&'*+-.^_|~");
		if (key.empty() || value.empty() || _headers.find(key) != _headers.end() ||
			key.find_first_not_of(allowedChars) != std::string::npos)
			return HTTP_BAD_REQUEST;
		_headers[key] = value;
	}catch(const std::exception &){
		return HTTP_BAD_REQUEST;
	}
	return HTTP_OK;
}

e_statusCode	PutRequest::checkHeaders(void){
	if (_headers.find("Host") == _headers.end())
		return (HTTP_BAD_REQUEST);
	if (_headers.find("Content-Length") == _headers.end() && _headers.find("Transfer-Encoding") == _headers.end())
		return (HTTP_LENGTH_REQUIRED);
	if (_headers.find("Transfer-Encoding") != _headers.end()){
		if (_headers.find("Transfer-Encoding")->second != "chunked")
			return HTTP_NOT_IMPLEMENTED;
		_isChunked = true;
	}
	if (!_isChunked){
		if (_headers["Content-Length"].find_first_not_of("0123456789") != std::string::npos)
			return HTTP_BAD_REQUEST;
		_contentLength = strtoll(_headers["Content-Length"].c_str(), NULL, 10);
	}
	_body.open(_fileName.c_str(), std::ios::out | std::ios::in | std::ios::trunc);
	if (!_body.is_open()){
		_parse.setParseState(Error);
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	_parse.setParseState(Body);
	return HTTP_OK;
}

e_statusCode	PutRequest::parseBody(std::string &line) {
	size_t	bytesToWrite = 0;
	try{
		if (!_isChunked){
			bytesToWrite = std::min(_contentLength, line.size());
			if (bytesToWrite + _bodyIndex > _contentLength)
				bytesToWrite = _contentLength - _bodyIndex;
			_body.write(line.c_str(), bytesToWrite);
			_bodyIndex += bytesToWrite;
			line.erase(0, bytesToWrite);
			if (_bodyIndex == _contentLength){
				_parse.setParseState(Done);
				_body.close();
			}
		}
		else {
			while (line.size() > 0 && _parse.getParseState() != Done){
				if (!_gotChunkLen) {
					if (line.substr(0, 2) == "\r\n")
				        line.erase(0, 2);
				    size_t endOfChunkSize = line.find("\r\n");
					if (endOfChunkSize == std::string::npos)
						return HTTP_BAD_REQUEST;
				    _chunkLen = strtoll(line.substr(0, endOfChunkSize).c_str(), NULL, 16);
				    line.erase(0, endOfChunkSize + 2);
					_gotChunkLen = true;		
				}
				if (_chunkLen == 0) {
				    _parse.setParseState(Done);
				    _body.close();
				    break;
				}
				bytesToWrite = std::min(_chunkLen, line.size());
				if (_bodyIndex + bytesToWrite > _chunkLen)
					bytesToWrite = _chunkLen - _bodyIndex;
				_body.write(line.c_str(), bytesToWrite);
				_bodyIndex += bytesToWrite;
				line.erase(0, bytesToWrite);
				if (_bodyIndex >= _chunkLen) {
					if (line.substr(0, 2) == "\r\n")
				        line.erase(0, 2);
				    _bodyIndex = 0;
					_gotChunkLen = false;
				}
			}
		}
	}catch(const std::exception &){
		_parse.setParseState(Error);
		return HTTP_BAD_REQUEST;
	}
	return HTTP_OK;
}

std::string&                        PutRequest::getFileName( void ) {
	return _fileName;
}

PutRequest::~PutRequest( void ){
	if (_body.is_open())
		_body.close();
	std::remove(_fileName.c_str());
}