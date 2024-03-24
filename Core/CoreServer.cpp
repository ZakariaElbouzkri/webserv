/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CoreServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nakebli <nakebli@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/03/08 02:35:06 by zel-bouz          #+#    #+#             */
/*   Updated: 2024/03/24 04:51:37 by nakebli          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CoreServer.hpp"

CoreServer*	CoreServer::_instance = NULL;

CoreServer::CoreServer( void ) {
}

CoreServer::~CoreServer( void ) {
	for (size_t i = 0; i < _servers.size(); i++)
	{
		delete _servers[i];
	}
}


CoreServer*	CoreServer::getCore( void )
{
	if (_instance == NULL)
		_instance = new CoreServer();
	return _instance;
}


int	CoreServer::_nfds( void ) {
	int ans = -1;
	for (size_t i = 0; i < _servers.size(); i++) {
		ans = std::max(ans, _servers[i]->fileno());
		for (size_t	j = 0; j < _servers[i]->_clients.size(); j++) {
			ans = std::max(ans, _servers[i]->_clients[j]->fileno());
			ans = std::max(ans, _servers[i]->_clients[j]->getCgiFd());
		}   
	}
	return ans;
}

void	CoreServer::_acceptNewClient( Server* server ) {
	sockaddr_in	info;
	socklen_t	len = 0;
	memset(&info, 0, sizeof(sockaddr_in));
	int ans = accept(server->fileno(), (sockaddr*)&info, &len);
	info.sin_port = server->getInfo().sin_port;
	if (ans == -1)
		throw std::runtime_error("accept() faild");
	if (fcntl(ans, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl() faild");
	info = server->_info;
	Client*	client = new Client(_selector, ans, info);
	server->_clients.push_back(client);
	std::cout << strTime() << " client at " << *client << " accepted" << std::endl;
}

void	CoreServer::init( void ) {
	std::set<unsigned int>	_ports = MainConf::getConf()->getAllPorts();
	for (std::set<unsigned int>::iterator it = _ports.begin(); it != _ports.end(); it++)
	{
		Server* server = new Server(_selector);
		try {
			server->init(HOST, *it);
			std::cout << strTime() << slog(" start listening on [%s:%d]", HOST, *it) << std::endl;
			_servers.push_back(server);
		} catch (std::exception & e) {
			delete server;
			std::cout << strTime() << slog(" faild to setup server on [%s:%d]", HOST, *it) << std::endl;
			sleep(1);
		}

	}
}

void	CoreServer::_purgeClient( Server* server, std::vector<Client*>::iterator& it )
{
	Client*	client;
	
	client = *it;
	it = server->_clients.erase(it);
	std::cout << strTime() << " client at " << *client << " diconnected " << std::endl;
	delete client;
}


void	CoreServer::_manageClients( Server* server )
{
	for (std::vector<Client*>::iterator it = server->_clients.begin(); it != server->_clients.end(); ) {
		Client* client = *it;
		if (_selector.isReadable(client->fileno())) {
			char buff[1024] = {0};
			int ret = ::recv(client->fileno(), buff, 1024, 0);
			if (ret == -1) {
				std::cout << strTime() << " recv() failed to read from client " << *client << std::endl; 
			}
			else if (ret == 0) {
				_purgeClient(server, it);
				continue;
			} else {
				client->readRequest(buff, ret);
			}
		}
		if (_selector.isReadable(client->getCgiFd())) {
			client->sendCgiRespo();
		}
		if (client->sendResponse()) {
			std::cout << "1\n";
			_purgeClient(server, it);
			continue;
		}
		if (currTime() - client->lastActive()  >= TIMEOUT) {
			_purgeClient(server, it);
			continue;
		}
		++it;
	}
}

void	CoreServer::run( void )
{
	if (_servers.empty()) {
		std::cerr << strTime() << " no server runing exiting...." << std::endl;
		return ;
	}

	while (true) {
		int maxfds = _nfds();

		if (_selector.select(maxfds + 1) == false)
			continue;
		
		for (size_t i = 0; i < _servers.size(); i++) {
			Server* server = _servers[i];
			if (_selector.isReadable(server->fileno()) &&  maxfds < 1000) {
				try {
					_acceptNewClient(server);
				} catch (std::exception & e) {
					std::cerr << strTime() << ": failed to accept new client: " << e.what() << std::endl;
				}
			}
			_manageClients(server);
		}
	}
}