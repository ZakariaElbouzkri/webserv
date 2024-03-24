/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetRequest.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nakebli <nakebli@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 21:59:15 by abizyane          #+#    #+#             */
/*   Updated: 2024/03/22 02:02:12 by nakebli          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "IRequest.hpp"

class ProcessRequest;

class	GetRequest : public IRequest{
	private:
		std::string							_method;
		std::string							_uri;
		std::string							_version;
		std::map<std::string, std::string>	_headers;
		std::fstream						_body;
		std::string							_fileName;
		bool								_hasBody;
		size_t								_bodyIndex;
		bool								_isChunked;
		size_t								_chunkLen;
		bool								_gotChunkLen;
		size_t								_contentLength;
		ProcessRequest&						_parse;

	public:
		GetRequest(std::string &method, std::string &uri, ProcessRequest& parse);

		e_statusCode						checkHeaders( void );
		e_statusCode						parseBody(std::string &line);
		e_statusCode						parseHeader(std::string &line);

		std::string							getMethod( void ) const;
		std::string							getUri( void ) const;
		std::map<std::string, std::string>	getHeaders( void ) const;
		std::vector<char>					getBody( void );
		ssize_t								getBodySize( void ) const;
		ProcessRequest&						getParse( void ) const;
		void								setUri( std::string uri );
		std::string&                        getFileName( void );

		~GetRequest( void );
};

#include "ProcessRequest.hpp"


