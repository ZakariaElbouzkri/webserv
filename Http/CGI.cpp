/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/25 18:30:35 by abizyane          #+#    #+#             */
/*   Updated: 2024/09/11 19:22:45 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#include "../utils/utils.hpp"

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
        delete [] _cgi_argv;
		exit(502);
    }
    delete _cgi_argv[0];
    delete _cgi_argv[1];
    delete [] _cgi_argv;
    fcntl(fd, F_SETFL, O_NONBLOCK);
    _selector.set(fd, Selector::RD_SET);
	return 0;
}

void    Response::_parseCgiResponse( std::string& output )
{
    size_t  pos = output.find("\n\n");
    if (pos == std::string::npos)
        throw HTTP_BAD_GATEWAY;
    std::stringstream headersPart(output.substr(0, pos));
    output = output.substr(pos + 2);
    
    std::string line;
    while (std::getline(headersPart, line)) {
        pos = line.find(":");
        if (pos == std::string::npos)
            throw HTTP_BAD_GATEWAY;
        _headers[trim(line.substr(0, pos))] = trim(line.substr(pos + 1));
    }
    if (_headers["Content-Type"] == "")
        throw HTTP_BAD_GATEWAY;
    if (_headers["Status"] == "")
        _headers["Status"] = "200 Ok";
    std::stringstream   ss(_headers["Status"]);
    int code; ss >> code;
	if (code < 100 || code > 599)
        throw HTTP_BAD_GATEWAY;
    _status = (e_statusCode)code;
    _headers.erase("Status");
    if (_headers["Content-Length"] == "")
        _headers["Content-Length"] = to_str(output.size());
    _file.close();
    std::remove(_responsefileName.c_str());
    _file.open(_responsefileName.c_str(), std::ios::in | std::ios::out | std::ios::trunc);
    if (!_file.is_open())
        throw HTTP_INTERNAL_SERVER_ERROR;
    _file.write(output.c_str(), output.size());
    _file.close();
    _file.open(_responsefileName.c_str(), std::ios::in | std::ios::out);
}

void    Response::readCgiFile(void) {
    std::string headers;
    std::string body;
    int status;
    int ret = waitpid(_cgi_pid, &status, WNOHANG);
    if (ret != _cgi_pid)
        return ;
    try {
        if ((WIFEXITED(status) && WEXITSTATUS(status) != 0) || WIFSIGNALED(status))
            throw HTTP_INTERNAL_SERVER_ERROR;
        std::ifstream cgiResponse(_responsefileName.c_str());
        if (!cgiResponse.is_open())
            throw HTTP_FORBIDDEN;
        std::stringstream buffer;
        buffer << cgiResponse.rdbuf();
        cgiResponse.close();
        std::string output = buffer.str();
        _parseCgiResponse(output);
    } catch(e_statusCode & status) {
        _status = status;
    } catch (const std::exception &) {
        _status = HTTP_INTERNAL_SERVER_ERROR;
    }
    int& fd = _parse->getCgiFd();
    _selector.unset(fd, Selector::RD_SET);
    close(fd);
    fd = -1;
	_buildResponse();
}
