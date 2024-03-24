#pragma once

#include "Selector.hpp"
#include "../Http/ProcessRequest.hpp"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define TIMEOUT 10

class	Client
{
	private:
		Selector&		_selector;
		int				sock;
		sockaddr_in		info;
		int				_cgi_fd;
		ProcessRequest	_processor;
		ssize_t			_bytesSent;
		time_t			_lastactive;
	
		inline	void	_updateLastActive( void ) {
			_lastactive = currTime();
		}

	public:
		Client( Selector& _selector, int sock, sockaddr_in info );
		~Client( void );
		bool		sendResponse( void );

		inline int fileno( void ) const {
			return sock;
		}

		inline int getCgiFd( void ) const {
			return _cgi_fd;
		}

		inline	sockaddr_in infos( void ) const {
			return info;
		}

		inline void	readRequest( char *buffer, int size ) {
			_processor.parseLine(buffer, size);
			_updateLastActive();
		}

		inline time_t	lastActive( void ) {
			return _lastactive;
		}
		
		inline void		sendCgiRespo() {
			_processor.getCgiResponse();
		}
		
		friend std::ostream&	operator<<( std::ostream& os, const Client& rhs );
};
