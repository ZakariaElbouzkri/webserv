#pragma once

#include <sys/select.h>
#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

class	Selector
{
	private:
		fd_set			readfds;
		fd_set			writefds;
		fd_set			rfds;
		fd_set			wfds;
	public:

		enum {
			WR_SET = 1 << 0, RD_SET = 1 << 1
		};

		Selector( void ) {
			FD_ZERO(&readfds);
			FD_ZERO(&writefds);
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
		}
		~Selector( void ) {};

		inline bool select( int nfds ) {
			rfds = readfds;
			wfds = writefds;
			int ret = ::select(nfds, &rfds, &wfds, NULL, NULL);
			if (ret == -1)
				perror("select");
			return ret > 0;
		}
		
		inline	void	set(int fd, short events ) {
			if (events & RD_SET)
				FD_SET(fd, &readfds);
			if (events & WR_SET)
				FD_SET(fd, &writefds);
		}

		inline	void	unset(int fd, short events) {
			if (events & RD_SET)
				FD_CLR(fd, &readfds);
			if (events & WR_SET)
				FD_CLR(fd, &writefds);
		}

		inline bool	isReadable( int fd) {
			return fd > 0 && FD_ISSET(fd, &rfds) != 0;
		}
		inline bool isWriteable( int fd ) {
			return fd > 0 && FD_ISSET(fd, &wfds) != 0;
		}
};