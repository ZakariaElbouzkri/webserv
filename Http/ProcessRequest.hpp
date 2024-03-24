/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessRequest.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 16:04:49 by abizyane          #+#    #+#             */
/*   Updated: 2024/02/13 16:39:58 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <sstream>
#include <algorithm>
#include "IRequest.hpp"
#include "GetRequest.hpp"
#include "PostRequest.hpp"
#include "DeleteRequest.hpp"
#include "../Core/Selector.hpp"

class Response;

typedef enum {
	Error,
	RequestLine,
	Headers,
	Body,
	Done
} e_parseState;

class ProcessRequest {
	private:
		std::string 	_requestBuffer;
		int				_port;
		e_parseState	_state;
		e_statusCode	_status;
		IRequest*		_request;
		Response*		_response;
		int&			_cgi_fd;
		bool			_good;
		std::string		_responseBuffer;
		Selector&		_selector;

		int				_pid;

		void			_parseRequestLine(std::string&	requestLine);
		void			_resetProcessor( void );

	public:
		ProcessRequest(int port, Selector& _selector, int& fd);

		void			parseLine(char *buffer, int size);
		bool			good( void );

		IRequest*		getRequest( void );
		Response*		getResponse( void );
		e_parseState	getParseState( void );
		e_statusCode	getStatusCode( void );

		void			setParseState(e_parseState state);
		void			setGood(bool good);
		void			resetRequest( void );
		int&			getCgiFd();
		bool			sent( void );
		const int&		getPort() const;
		void			getCgiResponse();

		~ProcessRequest();
};

#include "Response.hpp"
/*

requestMethod = GET | POST | DELETE

class   RequestFactory:


parseFirstLine();

class   IRequest:
	void    parse( void ) = 0;

class GETRequest extends IRequest:
	void    parse( void ) = 0;

*/