/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ResponseUtils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nakebli <nakebli@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/01 18:15:20 by abizyane          #+#    #+#             */
/*   Updated: 2024/03/22 11:51:31 by nakebli          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

void	Response::initMaps(){
	static bool		initialized = false;
	if (!initialized){
		_statusMap[HTTP_CONTINUE] = "100 Continue";
		_statusMap[HTTP_SWITCHING_PROTOCOLS] = "101 Switching Protocols";
		_statusMap[HTTP_OK] = "200 OK";
		_statusMap[HTTP_CREATED] = "201 Created";
		_statusMap[HTTP_ACCEPTED] = "202 Accepted";
		_statusMap[HTTP_NON_AUTHORITATIVE_INFORMATION] = "203 Non-Authoritative Information";
		_statusMap[HTTP_NO_CONTENT] = "204 No Content";
		_statusMap[HTTP_RESET_CONTENT] = "205 Reset Content";
		_statusMap[HTTP_PARTIAL_CONTENT] = "206 Partial Content";
		_statusMap[HTTP_MULTIPLE_CHOICES] = "300 Multiple Choices";
		_statusMap[HTTP_MOVED_PERMANENTLY] = "301 Moved Permanently";
		_statusMap[HTTP_FOUND] = "302 Found";
		_statusMap[HTTP_SEE_OTHER] = "303 See Other";
		_statusMap[HTTP_NOT_MODIFIED] = "304 Not Modified";
		_statusMap[HTTP_USE_PROXY] = "305 Use Proxy";
		_statusMap[HTTP_TEMPORARY_REDIRECT] = "307 Temporary Redirect";
		_statusMap[HTTP_BAD_REQUEST] = "400 Bad Request";
		_statusMap[HTTP_UNAUTHORIZED] = "401 Unauthorized";
		_statusMap[HTTP_PAYMENT_REQUIRED] = "402 Payment Required";
		_statusMap[HTTP_FORBIDDEN] = "403 Forbidden";
		_statusMap[HTTP_NOT_FOUND] = "404 Not Found";
		_statusMap[HTTP_METHOD_NOT_ALLOWED] = "405 Method Not Allowed";
		_statusMap[HTTP_NOT_ACCEPTABLE] = "406 Not Acceptable";
		_statusMap[HTTP_PROXY_AUTHENTICATION_REQUIRED] = "407 Proxy Authentication Required";
		_statusMap[HTTP_REQUEST_TIMEOUT] = "408 Request Timeout";
		_statusMap[HTTP_CONFLICT] = "409 Conflict";
		_statusMap[HTTP_GONE] = "410 Gone";
		_statusMap[HTTP_LENGTH_REQUIRED] = "411 Length Required";
		_statusMap[HTTP_PRECONDITION_FAILED] = "412 Precondition Failed";
		_statusMap[HTTP_REQUEST_ENTITY_TOO_LARGE] = "413 Request Entity Too Large";
		_statusMap[HTTP_REQUEST_URI_TOO_LONG] = "414 Request-URI Too Long";
		_statusMap[HTTP_UNSUPPORTED_MEDIA_TYPE] = "415 Unsupported Media Type";
		_statusMap[HTTP_REQUESTED_RANGE_NOT_SATISFIABLE] = "416 Requested Range Not Satisfiable";
		_statusMap[HTTP_EXPECTATION_FAILED] = "417 Expectation Failed";
		_statusMap[HTTP_INTERNAL_SERVER_ERROR] = "500 Internal Server Error";
		_statusMap[HTTP_NOT_IMPLEMENTED] = "501 Not Implemented";
		_statusMap[HTTP_BAD_GATEWAY] = "502 Bad Gateway";
		_statusMap[HTTP_SERVICE_UNAVAILABLE] = "503 Service Unavailable";
		_statusMap[HTTP_GATEWAY_TIMEOUT] = "504 Gateway Timeout";
		_statusMap[HTTP_VERSION_NOT_SUPPORTED] = "505 HTTP Version Not Supported";
		
		_mimeMap["html"] = "text/html";
		_mimeMap["htm"] = "text/html";
		_mimeMap["css"] = "text/css";
		_mimeMap["js"] = "text/javascript";
		_mimeMap["js"] = "application/javascript";
		_mimeMap["jpg"] = "image/jpeg";
		_mimeMap["jpeg"] = "image/jpeg";
		_mimeMap["png"] = "image/png";
		_mimeMap["gif"] = "image/gif";
		_mimeMap["bmp"] = "image/bmp";
		_mimeMap["ico"] = "image/x-icon";
		_mimeMap["svg"] = "image/svg+xml";
		_mimeMap["mp3"] = "audio/mpeg";
		_mimeMap["mp4"] = "video/mp4";
		_mimeMap["webm"] = "video/webm";
		_mimeMap["ogg"] = "audio/ogg";
		_mimeMap["wav"] = "audio/wav";
		_mimeMap["avi"] = "video/x-msvideo";
		_mimeMap["mpeg"] = "video/mpeg";
		_mimeMap["txt"] = "text/plain";
		_mimeMap["pdf"] = "application/pdf";
		_mimeMap["doc"] = "application/msword";
		_mimeMap["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
		_mimeMap["xls"] = "application/vnd.ms-excel";
		_mimeMap["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
		_mimeMap["ppt"] = "application/vnd.ms-powerpoint";
		_mimeMap["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
		_mimeMap["zip"] = "application/zip";
		_mimeMap["rar"] = "application/x-rar-compressed";
		_mimeMap["tar"] = "application/x-tar";
		_mimeMap["gz"] = "application/gzip";
		_mimeMap["7z"] = "application/x-7z-compressed";
		_mimeMap["exe"] = "application/x-msdownload";
		_mimeMap["swf"] = "application/x-shockwave-flash";
		_mimeMap["iso"] = "application/x-iso9660-image";
		_mimeMap["img"] = "application/x-iso9660-image";
		_mimeMap["flv"] = "video/x-flv";
		_mimeMap["wmv"] = "video/x-ms-wmv";
		_mimeMap["webp"] = "image/webp";
		_mimeMap["ttf"] = "font/ttf";
		_mimeMap["otf"] = "font/otf";
		_mimeMap["woff"] = "font/woff";
		_mimeMap["woff2"] = "font/woff2";
		_mimeMap["eot"] = "application/vnd.ms-fontobject";
		_mimeMap["json"] = "application/json";
		_mimeMap["xml"] = "application/xml";
		_mimeMap[""] = "application/octet-stream";
		_mimeMap["cpp"] = "text/x-c";
		_mimeMap["hpp"] = "text/x-c";
		_mimeMap["java"] = "text/x-java-source";
		_mimeMap["py"] = "text/x-python";
		_mimeMap["sh"] = "text/x-shellscript";
		_mimeMap["pl"] = "text/x-perl";
		_mimeMap["rb"] = "text/x-ruby";
		_mimeMap["cs"] = "text/x-csharp";
		_mimeMap["go"] = "text/x-go";
		_mimeMap["rs"] = "text/x-rust";
		_mimeMap["swift"] = "text/x-swift";
		_mimeMap["kt"] = "text/x-kotlin";
		_mimeMap["ts"] = "text/x-typescript";
		_mimeMap["php"] = "text/x-php";
		_mimeMap["lua"] = "text/x-lua";

		initialized = true;
	}
}

std::string		getExtension(const std::string &fileName){
	size_t		pos = fileName.find_last_of(".");
	if (pos == std::string::npos || pos == 0)
		return "";
	return fileName.substr(pos + 1);
}

void	Response::_openFile(std::string &fileName, int param){
	if (param == 0){
		if (access(fileName.c_str(), F_OK) == -1)
			throw ResponseException(HTTP_NOT_FOUND);
		if (access(fileName.c_str(), R_OK) == -1)
			throw ResponseException(HTTP_FORBIDDEN);
		_file.open(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::out);	
	}
	else
		_file.open(fileName.c_str(), std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
	if (!_file.is_open())
		throw ResponseException(HTTP_INTERNAL_SERVER_ERROR);
}