/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   PostRequest.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 22:03:16 by abizyane          #+#    #+#             */
/*   Updated: 2024/03/17 22:04:08 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "PostRequest.hpp"

PostRequest::PostRequest(std::string &method, std::string &uri, ProcessRequest& parse)
    : _method(method), _uri(uri), _version("HTTP/1.1"), _parse(parse){
	_contentLength = 0;
	_bodyIndex = 0;
	_isChunked = false;
	_chunkLen = 0;
	_gotChunkLen = false;
	_fileName = ".requestbody";
}

std::string		PostRequest::getMethod( void ) const{
	return _method;
}

std::string		PostRequest::getUri( void ) const{
	return _uri;
}

std::map<std::string, std::string>	PostRequest::getHeaders( void ) const{
	return _headers;
}

std::vector<char>	PostRequest::getBody( void ) {
	struct stat	s;
	stat(_fileName.c_str(), &s);

	std::vector<char> buffer(s.st_size);
	_body.open(_fileName.c_str(), std::ios::in);
	_body.read(buffer.data(), s.st_size);
	_body.close();
	return buffer;
}

ProcessRequest&	PostRequest::getParse( void ) const{
	return _parse;
}

e_statusCode	PostRequest::parseHeader(std::string &line){
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

e_statusCode	PostRequest::checkHeaders(void){
	if (_headers.find("Host") == _headers.end())//|| _headers.find("Content-Type") == _headers.end())
		return (HTTP_BAD_REQUEST);
	if (_headers.find("Content-Length") == _headers.end() && _headers.find("Transfer-Encoding") == _headers.end())
		return (HTTP_BAD_REQUEST);
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
	std::srand(std::time(0));
	for (size_t i = 0; i < 5; i++)
		_fileName += to_str(std::rand());
	_body.open(_fileName.c_str(), std::ios::out | std::ios::in | std::ios::trunc);
	if (!_body.is_open()){
		_parse.setParseState(Error);
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	_parse.setParseState(Body);
	return HTTP_OK;
}

e_statusCode	PostRequest::parseBody(std::string &line){
	std::stringstream ss(line);
	std::string	str;
	size_t	bytesToWrite = 0;
	try{
		if (!_isChunked){
			bytesToWrite = std::min(_contentLength, line.size());
			_body.write(line.c_str(), bytesToWrite);
			_bodyIndex += bytesToWrite;
			if(_bodyIndex == _contentLength){
				_parse.setParseState(Done);
				_body.close();	
			}
		}
		else {
			while (line.size() > 0 && _parse.getParseState() != Done){
				std::cout << "line: " << line << std::endl;
				// std::cout << "chunkLen: " << _chunkLen << " _bodyIndex: " << _bodyIndex << std::endl;
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
				std::cout << "_chunkLen: " << _chunkLen << " _bodyIndex: " << _bodyIndex << " line.size: " << line.size() << std::endl;
				std::cout << "bytesToWrite: " << bytesToWrite << std::endl;
				if (_bodyIndex + bytesToWrite > _chunkLen)
					bytesToWrite = _chunkLen - _bodyIndex;
				_body.write(line.c_str(), bytesToWrite);
				_bodyIndex += bytesToWrite;
				line.erase(0, bytesToWrite);
				if (_bodyIndex == _chunkLen) {
					if (line.substr(0, 2) == "\r\n")
				        line.erase(0, 2);
				    _bodyIndex = 0;
					_chunkLen = false;
				}
				// usleep(100);
			}
		}
	}catch(const std::exception &){
		_parse.setParseState(Error);
		return HTTP_BAD_REQUEST;
	}
	line.clear();
	return HTTP_OK;
}

PostRequest::~PostRequest( void ){
	if (_body.is_open())
		_body.close();
	std::remove(_fileName.c_str());
}