/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zel-bouz <zel-bouz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/08 02:39:26 by zel-bouz          #+#    #+#             */
/*   Updated: 2024/03/08 04:14:45 by zel-bouz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client( Selector& _selector, int sock, sockaddr_in info ) : _selector(_selector), sock(sock), info(info), _processor(htons(info.sin_port)){
	_selector.set(sock, Selector::WR_SET | Selector::RD_SET);
	fd[0] = fd[1] = -1;
	_bytesSent = 0;
}

Client::~Client( void ) {
	_selector.unset(sock, Selector::WR_SET | Selector::RD_SET);
	close(sock);
	close(fd[1]);
	close(fd[0]);
}


bool		Client::sendResponse( void ) {
	if (_processor.good()){
		std::string response = _processor.getResponse()->GetResponse(_bytesSent);
		// one bytes not readed in file ??
		// check GET /index.html HTTP/1.1
		// print the response
		_bytesSent = ::send(sock, response.c_str(), response.size(), 0);
		return (_processor.getResponse()->sent());
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