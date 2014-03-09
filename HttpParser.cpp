/*
 * http_parser.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */
#include "HttpParser.h"


HttpParser::HttpParser(int connfd):
	 sockfd_(0),
	 in_(),
	 out_(),
	 isNewLine_(true),
	 wordst_(W_TAG),
	 linest_(L_REQ)
{
	clearline();
}

HttpParser::HttpParser(HttpParser * const & h):
	 sockfd_(0),
	 in_(),
	 out_(),
	 isNewLine_(true),
	 wordst_(W_TAG),
	 linest_(L_REQ)
{}

HttpParser::~HttpParser(){}


void HttpParser::process(int tid){

	parse();
	fd_mod_out(m_epollfd, sockfd_);
}

void HttpParser::response(){

	fd_mod_in(m_epollfd, sockfd_);
}

void HttpParser::parse(){

	string s;
    int loop = 0;
    while(in_ >> s){
    	++loop;
//    	int npos = in_.tellg();
//    	int len = s.length();

    	//Set the WORD_STATE, peek next character to see if the word or line is complete.
    	char c = in_.peek();
    	switch( c ){
			case 0x20: // space
				wordst_ = W_CONTENT;
				if(isNewLine_){
					wordst_ = W_TAG;
					isNewLine_ = false;
				}
				break;
			case 0xd: // \r
			case 0xa: // \n
				wordst_ = W_CONTENT;
				isNewLine_ = true;
				break;
			case -1: // eof
				wordst_ = W_CLOSE;
				break;
			default:
				cout << "something bad happened" << endl;
				break;

    	}//switch

    	int wordcounter;
		switch(wordst_){
			case W_TAG:
			{
				wordcounter = 0;
				if(s == "GET"){
					linest_ = L_REQ;
					m_method = GET;
				}
				else if(s == "HEAD"){
					linest_ = L_REQ;
					m_method = HEAD;
				}
				else if(s == "TRACE"){
					linest_ = L_REQ;
					m_method = TRACE;
				}
				else if(s == "Connection:"){
					linest_ = L_CONNECTION;
				}
				else if(s == "User-Agent:"){
					linest_ = L_AGENT;
				}
				else if(s == "Host:"){
					linest_ = L_HOST;
				}
				else{
					linest_ = L_UNKNOWN;
				}
				break;
			}
			case W_CONTENT:
			{
				++wordcounter;
				switch(linest_){
					case L_REQ:
					{
						if(wordcounter == 1){
							m_url = s;
						}
						else if(wordcounter == 2){
							m_version = s;
						}
						break;
					}
					case L_CONNECTION:
					{
						if(s == "close"){
							m_conn = CLOSE;
						}
						else if(s == "keep-alive"){
							m_conn = KEEPALIVE;
						}
						break;
					}
					case L_HOST:
					{
						m_host += s;
						break;
					}
					case L_AGENT:
					{
						m_agent += s;
						m_agent += " ";
						break;
					}
					case L_UNKNOWN:
						break;
				}
				break;
			}
			case W_CLOSE:
			{
				cout << "header is read completely!" << endl;
				break;
			}
		}
    }//while

}

void HttpParser::writeResHead(){

}

string HttpParser::geturlfile(){
	string s;
	stringstream ss(home_dir);
	bool isdir = false;
	while(strm_recv >> s){
		if(isdir){
			if(s == "/"){
				ss << string("index");
			}
			else{
				ss << s;
			}
			return ss.str();
		}
		if(s == "GET"){
			isdir = true;
		}
	}
	return string();
}
