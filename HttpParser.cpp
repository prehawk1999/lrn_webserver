/*
 * http_parser.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */
#include "HttpParser.h"


HttpParser::HttpParser(int connfd):
	 sockfd_(connfd),
	 in_(connfd),
	 out_(connfd),
	 isNewLine_(true),
	 wordst_(W_TAG),
	 linest_(L_REQ),
	 word_(),
	 contentcount_(0)
{
	fd_add(m_epollfd, sockfd_);
}

HttpParser::HttpParser(HttpParser * const & h):
	 sockfd_(h->sockfd_),
	 in_(h->sockfd_),
	 out_(h->sockfd_),
	 isNewLine_(h->isNewLine_),
	 wordst_(h->wordst_),
	 linest_(h->linest_),
	 word_(),
	 contentcount_(0)
{}

HttpParser::~HttpParser(){}


void HttpParser::process(int tid){

	parse();
	fd_mod_out(m_epollfd, sockfd_);
}

void HttpParser::response(){

	fd_mod_in(m_epollfd, sockfd_);
}

void HttpParser::switchLine(){
	contentcount_ = 0;
	if(word_ == "GET"){
		linest_ = L_REQ;
		m_method = GET;
	}
	else if(word_ == "HEAD"){
		linest_ = L_REQ;
		m_method = HEAD;
	}
	else if(word_ == "TRACE"){
		linest_ = L_REQ;
		m_method = TRACE;
	}
	else if(word_ == "Connection:"){
		linest_ = L_CONNECTION;
	}
	else if(word_ == "User-Agent:"){
		linest_ = L_AGENT;
	}
	else if(word_ == "Host:"){
		linest_ = L_HOST;
	}
	else{
		linest_ = L_UNKNOWN;
	}
}

void HttpParser::fillStates(){

	++contentcount_;
	switch(linest_){
		case L_REQ:
		{
			if(contentcount_ == 1){
				m_url = word_;
				if(m_url != "/"){
					m_url_file = home_dir + m_url;
				}
				else{
					m_url_file = home_dir + string("/index");
				}
			}
			else if(contentcount_ == 2){
				m_version = word_;
			}
			break;
		}
		case L_CONNECTION:
		{
			if(word_ == "close"){
				m_conn = CLOSE;
			}
			else if(word_ == "keep-alive"){
				m_conn = KEEPALIVE;
			}
			break;
		}
		case L_HOST:
		{
			m_host += word_;
			break;
		}
		case L_AGENT:
		{
			m_agent += word_;
			m_agent += " ";
			break;
		}
		case L_UNKNOWN:
			break;
	}
}

void HttpParser::setHttpState(){

	switch(wordst_){
		case W_TAG:
		{
			switchLine();
			break;
		}
		case W_CONTENT:
		{
			fillStates();
			break;
		}
		case W_BAD:
		{
			cout << "header is incomplete!" << endl;
			break;
		}
	}
}

void HttpParser::parse(){

    while(in_ >> word_){
		int npos = in_.tellg();
		int len = word_.length();

    	//Set the WORD_STATE, peek next character to see if the word or line is complete.
    	char c = in_.peek();
    	switch( c ){
    		case 0x20: // space
    			cout << word_ << " ";
    			wordst_ = W_CONTENT;
    			if(isNewLine_){
    				wordst_ = W_TAG;
    				isNewLine_ = false;
    			}
    			break;
    		case 0xd: // \r
    		case 0xa: // \n
    			cout << word_ << endl;
    			wordst_ = W_CONTENT;
    			isNewLine_ = true;
    			break;
    		case -1: // eof
    			wordst_ = W_BAD;
    			in_.seekg(npos - len, std::ios_base::beg);
    			break;
    		default:
    			cout << "something bad happened" << endl;
    			break;

    	}//switch
    	setHttpState();
    }
    cout << endl;
}


