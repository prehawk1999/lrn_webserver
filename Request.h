/*
 * client.h
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */
/*
 *
 *
 *
 * */

#ifndef CLIENT_H_
#define CLIENT_H_
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdio>

#include "config.hpp"
#include "utils.h"

class Request
{
public:
	static int m_epollfd;
	static int m_user_count;

public:
	Request();
	Request(Request * const & hr);
	virtual ~Request();

	void init(int connfd );
	void process(int tid);
	void response();
	void destroy();

protected:
	void zerobuffer();

	int sockfd_;

};



#endif /* CLIENT_H_ */
