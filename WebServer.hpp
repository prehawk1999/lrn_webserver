/*
 * serverIO.h
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */

#ifndef SERVERIO_H_
#define SERVERIO_H_

#include <syslog.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/logic/tribool.hpp>
#include <vector>
#include <string.h>
#include <errno.h>
#include <clocale>

#include "threadpool.hpp"
#include "utils.h"
#include "config.hpp"

namespace ws{
	using std::vector;
	using boost::shared_ptr;
	using boost::make_shared;
}

using namespace ws;


// T is the type of request, whose ptr form can be push to request queue.
// *MUST* implement process();  response(); destroy(); method by T class.
// *SHOULD HAVE* m_epollfd static member.
// main thread.
template<typename T>
class WebServer
{
public:
	typedef shared_ptr<T> 							req_t;
	typedef vector< req_t > 						reqpool_t;
	typedef typename vector< req_t >::iterator 		req_iter;

public:
	// @ip address, @port, @thread number create, @max request number.
	explicit WebServer(const char * ip, int port, int threadnum, int reqqueue);
	int initialize(const char * ip, int port);
 	~WebServer();
	void run();
private:
	void loopwait();
private:
	threadpool< req_t > 		m_threadpool;
	reqpool_t 					m_reqpool;

private:
	int m_listenfd;
	int m_epollfd;
	epoll_event m_events[MAX_EVENTS];

};

template<typename T>
WebServer<T>::~WebServer(){
}

template<typename T>
WebServer<T>::WebServer(const char * ip, int port, int threadnum, int reqqueue):
	m_threadpool( threadpool< req_t >(threadnum, reqqueue) ), m_reqpool(MAX_USER){
	initialize(ip, port);
}

template<typename T>
int WebServer<T>::initialize(const char * ip, int port){
	m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
	assert( m_listenfd >= 0);

    struct linger tmp = { 1, 0 };
    setsockopt( m_listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof( tmp ) );

    sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, (const char *)ip, &address.sin_addr);
    address.sin_port = htons(port);

    int ret = 0;
    ret = bind(m_listenfd, (sockaddr *)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(m_listenfd, MAX_USER/2);
    assert(ret != -1);

    m_epollfd = epoll_create(5);
    assert(m_epollfd != -1);

    fd_add(m_epollfd, m_listenfd);
    setnonblocking(m_listenfd);

    T::m_epollfd = m_epollfd;
    return 0;
}

template<typename T>
void WebServer<T>::run(){
	m_threadpool.run();
	loopwait();
}


template<typename T>
void WebServer<T>::loopwait(){
	int number;
	//T * model_T = new T(0);
	bool newfd = true;
	while(1){
		number = epoll_wait(m_epollfd, m_events, MAX_EVENTS, -1);
		assert(( number >= 0 ) || ( errno == EINTR ));

		for(int i=0; i<number; ++i){
			int sockfd = m_events[i].data.fd;
			if(sockfd == m_listenfd){
				fd_mod_in(m_epollfd, m_listenfd);
				sockaddr_in client_addr;
				socklen_t addr_len = sizeof(client_addr);
				int connfd = accept(m_listenfd, (sockaddr *)&client_addr, &addr_len);
				m_reqpool[connfd] = shared_ptr<T>(new T(connfd));
				fd_add(m_epollfd, connfd);
				setnonblocking(connfd);
				newfd = true;
			}
			else if( m_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){

			}
			else if( m_events[i].events & EPOLLIN ){
				if(newfd){
					m_threadpool.append( m_reqpool[sockfd], true);
					newfd = false;
				}
				else{
					m_threadpool.append( m_reqpool[sockfd], false );
				}
			}
			else if( m_events[i].events & EPOLLOUT ){ //todo
				fd_rmv(m_epollfd, sockfd);
				close(sockfd);
				m_reqpool[sockfd].reset();
			}
			else{
				continue;
			}
		}
	}
}
#endif /* SERVERIO_H_ */
