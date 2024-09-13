/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MethodsUtils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/25 18:29:39 by abizyane          #+#    #+#             */
/*   Updated: 2024/09/13 14:48:16 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

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
	else
		return;
	struct stat s;
	if (stat(resource.c_str(), &s) == -1)
		return;
	resource += "/" + std::string(timestamp);
	if (extension != "")
		resource += "." + extension;
	resource = normPath(resource);
	if (extension != "" && _mimeMap[extension] == "")
		throw Response::ResponseException(HTTP_UNSUPPORTED_MEDIA_TYPE);
	return;
}

void	Response::_handleRange(){
	try {
		if (_request->getHeaders()["Range"] != "") {
			std::string range = _request->getHeaders()["Range"];
			ssize_t length = strtoll(_headers["Content-Length"].c_str(), NULL, 10);
			ssize_t start = strtoll(range.substr(range.find("=") + 1, range.find("-")).c_str(), NULL, 10);
			range.erase(0, range.find("-") + 1);
			ssize_t end = length - 1;
			if (start >= end)
				end = length - start;
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
	if (cookie != ""){
		std::vector<std::pair<std::string, std::string> >	cookies;
		std::vector<std::string>	tmp;
		size_t		pos = cookie.find_first_of("; ");
		if (pos != std::string::npos)
			while (pos != std::string::npos){
				tmp.push_back(cookie.substr(0, pos));
				cookie.erase(0, pos + 2);
				pos = cookie.find_first_of("; ");
				if (pos == std::string::npos && cookie != "")
					tmp.push_back(cookie);
			}
		else
			tmp.push_back(cookie);
		for (size_t i = 0; i < tmp.size(); i++){
			size_t			index = tmp[i].find("=");
			std::string		key;
			if (index != std::string::npos){
				key = tmp[i].substr(0, index);
				tmp[i].erase(0, index + 1);
			}
			else{
				key = tmp[i];
				tmp[i] = "";
			}
			cookies.push_back(std::make_pair(key, tmp[i]));
		}
		std::vector<std::pair<std::string, std::string> > c;
		bool	f = false;
		for (size_t i = 0; i < cookies.size(); i++){
			if (f && cookies[i].second != "")
				_cookies.push_back(c);
			if (cookies[i].second != ""){
				c.push_back(cookies[i]);
				f = true;
			}
			if (cookies[i].second == "")
				c.push_back(cookies[i]);
			if (i == cookies.size() - 1)
				_cookies.push_back(c);
		}
	}
}

std::string Response::_autoIndex(const std::string& dirName) {
    std::string htmlPage;

    htmlPage += "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
    htmlPage += "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css\" integrity=\"sha512-DTOQO9RWCH3ppGqcWaEA1BIZOC6xxalwEsw9c2QQeAIftl+Vegovlnee1c9QX4TctnWMn13TZye+giMm8e2LwA==\" crossorigin=\"anonymous\" referrerpolicy=\"no-referrer\" />\n";
    htmlPage += "<title>Index of " + dirName + "</title><style> body { font-family: Arial, sans-serif; } .fa-file { color: #85c3ec; padding-right: 15px;} a { text-decoration: none; color: #094f7e; } ";
    htmlPage += "i { color: #bea307; padding-right: 10px; } table { border-collapse: collapse; width: 100%; } th, td { padding: 10px 20px; text-align: left; } ";
    htmlPage += "th { background-color: #ffffff; } tr:nth-child(even) { background-color: #f8f7f7; } tr:hover { background-color: #ddd; } td { cursor: pointer; } </style> </head> ";
    htmlPage += "<body><h1>Index of " + dirName + ": </h1><hr><h2 style=\"padding: 10px;\"><a href=\"../\"><i class=\"fa-solid fa-backward\"></i> Parent Directory</a></h2><table><tr><th>Name</th><th>Last Modified</th><th>Size</th></tr>";

    DIR* dir;
    if ((dir = opendir(dirName.c_str())) != NULL) {
        struct dirent* ent;
        while ((ent = readdir(dir)) != NULL) {
            std::string fileName = ent->d_name;
            if (fileName == "." || fileName == "..")
                continue;
            std::string filePath = dirName + "/" + fileName;
            struct stat statbuff;
            if (stat(filePath.c_str(), &statbuff) != -1) {
                char buffer[30];
                strftime(buffer, 30, "%a, %d %b %Y %H:%M:%S %Z", gmtime(&statbuff.st_mtime));
                std::string lastModified = buffer;
                std::string fileSize = to_str(statbuff.st_size);
                if (S_ISDIR(statbuff.st_mode)) {
                    htmlPage += "<tr><td><a href=\"" + fileName + "/\"><i class=\"fa-solid fa-folder\"></i> " + fileName + "/</a></td>";
                } else {
                    htmlPage += "<tr><td><a href=\"" + fileName + "\"><i class=\"fa-solid fa-file\"></i> " + fileName + "</a></td>";
                }
                htmlPage += "<td>" + lastModified + "</td><td>" + fileSize + "</td></tr>";
            }
        }
        closedir(dir);
    } else {
        htmlPage += "<tr><td colspan=\"3\"><strong>Error:</strong> Couldn't open the directory: " + dirName  + "</td></tr>";
    }
    htmlPage += "</table></body></html>";
    _responsefileName += ".html";
    _openFile(_responsefileName, 1);
    _file << htmlPage;
    _file.close();

    return _responsefileName;
}

