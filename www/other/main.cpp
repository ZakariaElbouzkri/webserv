/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zel-bouz <zel-bouz@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/12 15:40:10 by zel-bouz          #+#    #+#             */
/*   Updated: 2024/03/18 22:05:03 by zel-bouz         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser/Parser.hpp"
// #include "Core/Server/Server.hpp"
#include "Core/CoreServer.hpp"
#include "Configuration/MainConf.hpp"
#include <signal.h>

void	typingEffect( const std::string& message )
{
	for (size_t i = 0; i < message.size(); i++) {
		std::cout << message[i];
		fflush(stdout);
		usleep(10000);
	}
	std::cout << std::endl;
}

void	handler( int sig )
{
	(void)sig;
	typingEffect("\b\b" + strTime() + " exiting server....");
	delete CoreServer::getCore();
	delete MainConf::getConf();
	exit(0);
}

int main()
{
	CoreServer	*core = CoreServer::getCore();

	signal(SIGINT, handler);
	signal(SIGQUIT, handler);
	signal(SIGPIPE, SIG_IGN);
	try {
		Parser	parser;
		parser.parse();
	} catch (std::exception & e) {
		std::cout << strTime() << " " << e.what() << std::endl;
	}
	core->init();
	core->run();
	delete CoreServer::getCore();
	delete MainConf::getConf();
}
