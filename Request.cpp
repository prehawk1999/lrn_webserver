/*
 * HttpReq.cpp
 *
 *  Created on: Mar 1, 2014
 *      Author: prehawk
 */

#include "Request.h"


int Request::m_user_count = 0;
int Request::m_epollfd = -1;

Request::Request(){}

Request::Request(Request * const & hr){}

Request::~Request(){}



void Request::init(int connfd ){

	sockfd_ = connfd;
	zerobuffer();
	strm_recv.rdbuf()->pubsetbuf(buffer_recv, sizeof(buffer_recv));
	strm_send.rdbuf()->pubsetbuf(buffer_send, sizeof(buffer_send));
	fd_add(m_epollfd, sockfd_);

}

void Request::zerobuffer(){

	memset(buffer_recv, '\0', sizeof buffer_recv);
	memset(buffer_send, '\0', sizeof buffer_send);

}

void Request::process(int tid){

	int bytesrecv;
	bytesrecv = recv(sockfd_, buffer_recv, sizeof buffer_recv, 0);
	if(bytesrecv == -1 or errno == EAGAIN){
		std::cerr << "HttpReq read() failed." << std::endl;
	}
	memcpy(buffer_send, buffer_recv, sizeof buffer_recv);
	fd_mod_out(m_epollfd, sockfd_);
}

void Request::response(){

	int bytessend;
	bytessend = send(sockfd_, buffer_send, strlen(buffer_send)+1, 0);
	if(bytessend == -1 or errno == EAGAIN){
		std::cerr << "HttpReq write() failed." << std::endl;
	}
	zerobuffer();

	fd_mod_in(m_epollfd, sockfd_);
}

void Request::destroy(){

}

