/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abizyane <abizyane@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/08 02:42:54 by zel-bouz          #+#    #+#             */
/*   Updated: 2024/09/13 14:44:58 by abizyane         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server (Selector& selector ) : _selector(selector) {
	memset(&_info, 0, sizeof(sockaddr_in));
};
	
void	Server::init( const std::string& host, int port ) {
	(void)host;
	_info.sin_addr.s_addr = INADDR_ANY;
	_info.sin_family = AF_INET;
	_info.sin_port = htons(port);
	_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (_sock == -1)
		throw std::runtime_error("socket() failed");
	int opt = 1;
	if (setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("setsockopt() failed");
	if (fcntl(_sock, F_SETFL, O_NONBLOCK) == -1)
		throw std::runtime_error("fcntl() failed");
	if (bind(_sock, (sockaddr*)&_info, sizeof(_info)) == -1)
		throw std::runtime_error("bind() failed");
	if (listen(_sock, SOMAXCONN) == -1)
		throw std::runtime_error("listen() failed");
	_selector.set(_sock, Selector::RD_SET);
}

Server::~Server( void ) {
	close(_sock);
	for (size_t i = 0; i < _clients.size(); i++) {
		delete _clients[i];
	}
}