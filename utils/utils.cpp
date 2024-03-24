/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nakebli <nakebli@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/13 09:51:28 by zel-bouz          #+#    #+#             */
/*   Updated: 2024/03/22 01:45:05 by nakebli          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "utils.hpp"
#include <stack>

/*
	""						=> "/"
	"//"					=> "/"
	"//home/../blog/"		=> "/blog"
	"/home/"				=> "/home"
	"./home"				=> "/home"
	"../home/"				=> "/home"
	"/home/../../../blog/"	=> "/blog"
*/

std::string normPath(const std::string& path) {
	std::string result;
	std::stack<std::string> s;

	s.push("/");
	for (size_t i = 0; i < path.size(); ++i) {
		if (path[i] == '/') {
            if (s.top() == ".")
                s.pop();
			if ((!s.empty() && s.top() == "/") || i + 1 == path.size())
				continue;
			else
				s.push("/");
		} else if (path[i] == '.') {
			if (i + 1 < path.size() && path[i + 1] == '.') {
				if (s.size() > 1) // Avoid popping the root "/"
					s.pop();
				i++;
			} else {
                s.push(".");
            }
		} else {
			std::string dir;
			while (i < path.size() && path[i] != '/') {
				dir += path[i];
				i++;
			}
			s.push(dir);
			--i;
		}
	}

	while (!s.empty()) {
		result = s.top() + result;
		s.pop();
	}

	return result;
}

unsigned long   getTime( void )
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec;
}

std::string slog(const char* format, ...)
{
    std::string result;
    va_list		args;

    va_start(args, format);
    for (size_t i = 0; format[i]; i++) {
        if (format[i] == '%' && strchr("sdf", format[i + 1])) {
            switch (format[++i]) {
                case 's': result.append(va_arg(args, const char*)); break;
                case 'd': result.append(to_str(va_arg(args, int))); break;
                case 'f': result.append(to_str(va_arg(args, double))); break;
                default: break;
            }
        } else {
            result += format[i];
        }
    }
    return va_end(args), result;
}


std::string	strTime()
{
    std::time_t currentTime;
    std::tm* localTime;
    char buffer[80];

    std::time(&currentTime);
    localTime = std::localtime(&currentTime);
    std::strftime(buffer, sizeof(buffer), "[%Y-%m-%d-%H:%M:%S]", localTime);
    return std::string(buffer);
}


time_t  currTime( void ) 
{
    time_t ans = time(NULL);
    return ans;
}

//  ========== EXTERNAL FUNCTIONNS ==========


std::string    getFileExtension(std::string request_uri) {
    std::string extension = \
    request_uri.substr(request_uri.find_last_of(".") + 1);
    extension = extension.substr(0, extension.find('?'));
    return extension;
}

std::string generateRandomFileName(int length) {
    const std::string allowed_chars = \
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::string file_name = "CGI_";
    std::srand(std::time(0));

    for (int i = 0; i < length; ++i) {
        int index = std::rand() % allowed_chars.size();
        file_name += allowed_chars[index];
    }

    return file_name;
}

std::string formKey(std::string key)
{
    std::string formedKey = key;
    for (size_t i = 0; i < formedKey.size(); i++)
    {
        if (formedKey[i] == '-')
            formedKey[i] = '_';
        else
            formedKey[i] = toupper(formedKey[i]);
    }
    return formedKey;
}