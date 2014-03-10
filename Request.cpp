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
	fd_add(m_epollfd, sockfd_);

}

void Request::zerobuffer(){


}

void Request::process(int tid){

	fd_mod_out(m_epollfd, sockfd_);
}

void Request::response(){


	fd_mod_in(m_epollfd, sockfd_);
}

void Request::destroy(){

}

