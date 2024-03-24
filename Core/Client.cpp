/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nakebli <nakebli@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/08 02:39:26 by zel-bouz          #+#    #+#             */
/*   Updated: 2024/03/24 05:17:45 by nakebli          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client( Selector& _selector, int sock, sockaddr_in info ) : _selector(_selector), sock(sock), info(info), 
	_processor(htons(info.sin_port), _selector, _cgi_fd) {
	_selector.set(sock, Selector::WR_SET | Selector::RD_SET);
	_cgi_fd = -1;
	_updateLastActive();
	_bytesSent = 0;
}

Client::~Client( void ) {
	_selector.unset(sock, Selector::WR_SET | Selector::RD_SET);
	_selector.unset(_cgi_fd, Selector::RD_SET);
	close(sock);
	close(_cgi_fd);
	_cgi_fd = -1;
}

bool		Client::sendResponse( void ) {
	if (_processor.good()){
		std::string response = _processor.getResponse()->GetResponse(_bytesSent);
		_bytesSent = 0;
		_bytesSent = ::send(sock, response.c_str(), response.size(), 0);
		if (_bytesSent == -1)
			_bytesSent = 0;
		_updateLastActive();
		return (_processor.sent());
	}
	return false;
}

std::ostream&	operator<<( std::ostream& os, const Client& rhs ) {
	char ip_address[INET_ADDRSTRLEN];
	if (inet_ntop(AF_INET, &(rhs.info.sin_addr), ip_address, INET_ADDRSTRLEN) != NULL) {
		os << ip_address << ":" << ntohs(rhs.info.sin_port);
	} else {
		os << "Failed to convert IP address";
	}
	return os;
}
