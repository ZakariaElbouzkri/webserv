/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/06 23:07:10 by abizyane          #+#    #+#             */
/*   Updated: 2024/03/24 22:23:56 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#pragma once

#include "ProcessRequest.hpp"
#include "../Configuration/MainConf.hpp"
#include "../Configuration/LocationConf.hpp"
#include "DefaultPages.hpp"
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctime>

typedef enum {
	ERROR,
	RESPONSE,
	BODY,
	DONE
} e_state;

class Response {
	private :
		static std::map<std::string, std::string>   _mimeMap;
		static std::map<e_statusCode, std::string>  _statusMap;
        IRequest*							_request;
		ProcessRequest*						_parse;
		LocationConf*						_location;
		ServerConf*							_server;
		bool								_good;
		e_statusCode						_status;
		e_state								_state;
		ssize_t								_bodyIndex;
		bool								_waitForCgi;
		std::string							_query;

		std::map<std::string, std::string>  _headers;
		std::fstream						_file;
		std::string							_responsefileName;
		std::string							_response;
		Selector&							_selector;
		// ======== For CGI ========
		std::string     					_query_string;
		char**          					_cgi_argv;
		pid_t           					_cgi_pid;
		std::string							_file_path;

		void								_buildResponse( void );
		void								_processGetResponse( void );
		void								_processPostResponse( void );
		void								_processDeleteResponse( void );
		void								_prepareResponse( void );
		void								_readFile(std::string resourse);
		void								_writeFile(std::string resource);
		void								_deleteFile(std::string resource);
		void								_handleRange( void );
		std::string							_autoIndex( const std::string& dirName );
		void								_openFile(std::string &fileName, int param);
		void								_handleCookies( void );
		std::vector<std::string>			_splitHeaderValue(std::string header);
		
		void								_getFileName(std::string &resource);
		void    							_setCGI_Arguments( void );
		void    							_initCGI();
		int    								_executeCGI( int& fd );
		void								_parseCgiHeaders( std::string headers );
		// void _printfile() {
		// 	    std::ifstream infile(_responsefileName);
    	// 		if (!infile.is_open()) {
    	// 		    std::cerr << "Failed to open the file." << std::endl;
    	// 		    return ;
    	// 		}
			
    	// 		std::cout << "===========---=========\n";
			
    	// 		std::string line;
    	// 		while (std::getline(infile, line)) {
    	// 		    std::cout << line << std::endl;
    	// 		}
    	// 		std::cout << "===========||=========\n";
    	// 		infile.close();
		// }

	public :
		Response(IRequest& request, ProcessRequest& parse, int port, Selector& _selector);
        static void							initMaps( void );
		bool			good( void );
		int				sent( void );

		IRequest*		getRequest( void );

		class ResponseException : public std::exception {
			private:
				e_statusCode	__status;

			public:
				ResponseException(e_statusCode status);
				e_statusCode 		getStatus( void );
				virtual const char* what() const throw();
		};

		std::string		GetResponse(size_t lastSent);
		int    			_getCGI_Response( void );

		~Response();
};

std::string		getExtension(const std::string &fileName);
