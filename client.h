/*
 * client.h
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */
/*
 * usage:
 * ServerIO ctrl = ServerIO("127.0.0.1", "6543");
 * Client cl = Client(connfd);
 * cl.process()
 * cl.
 *
 *
 * */

#ifndef CLIENT_H_
#define CLIENT_H_
#include <sys/epoll.h>
#include "http_parser.h"

class Client
{
public:



private:
	int m_epollfd;
	HttpParser * hpvec;
};



#endif /* CLIENT_H_ */
