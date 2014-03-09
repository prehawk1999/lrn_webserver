/*
 * timer_test.cpp
 *
 *  Created on: Feb 11, 2014
 *      Author: prehawk
 */
#include "WebServer.hpp"
#include "HttpParser.h"
#include "Request.h"
#include "IOwrapper.hpp"


#include <iostream>
#include <sstream>
#include <ios>




int main(int argc, char * argv[])
{

	WebServer<HttpParser> wb = WebServer<HttpParser>("127.0.0.1", 6543, 2, 10000);
	wb.run();

//    char head[] 	= "GET / HTTP1.1\r\nConnection: close\r\nUser-Agent: Wget/1.12 (linux-gnu)\r\nHost: www.baidu.com\r\n\r\n";
//    char head2[]    = "GET / HTTP1.1\r\nConnection: close\r\nUser-Agent: Wget/1.12 (linux-gnu)\r\nHost: www.baidu.com\r\n";
//    char inhead[] 	= "GET / HTTP1.1\r\nConnecti";
//    char inhead2[] 	= "GET / HTTP1.1\r\nConnection: close\r";
//    char inhead3[] 	= "GET / HTTP1.1\r\nConnection: clos";
//
//    bool isNewLine = true  ;
//    enum WORD_STATE{WORD_TAG, WORD_CONTENT, WORD_CLOSE } 		wordst = WORD_CONTENT;
//    enum LINE_STATE{L_REQ, L_CONNECTION, L_HOST, L_AGENT, L_UNKNOWN}	linest = L_REQ;
//
//	enum METHOD{GET, HEAD, TRACE}										m_method = GET;
//	enum CONNECTION{KEEPALIVE, CLOSE}									m_conn   = CLOSE;
//	enum HTTP_STATUS{_100, _200, _400, _403, _404, _500}				m_httpst = _200;
//
//	string	m_url;
//	string	m_version;
//	string  m_agent;
//	string  m_host;
//
//	io::stream<My_InOut> 	in(inhead3, sizeof inhead3);
//
//	string s;
//    int loop = 0;
//    while(in >> s){
//    	++loop;
//    	int npos = in.tellg();
//    	int len = s.length();
//
//    	//Set the WORD_STATE, peek next character to see if the word or line is complete.
//    	char c = in.peek();
//    	switch( c ){
//			case 0x20: // space
//				wordst = WORD_CONTENT;
//				if(isNewLine){
//					wordst = WORD_TAG;
//					isNewLine = false;
//				}
//				break;
//			case 0xd: // \r
//			case 0xa: // \n
//				wordst = WORD_CONTENT;
//				isNewLine = true;
//				break;
//			case -1: // eof
//				wordst = WORD_CLOSE;
//				break;
//			default:
//				cout << "something bad happened" << endl;
//				break;
//
//    	}//switch
//
//    	int wordcounter;
//		switch(wordst){
//			case WORD_TAG:
//			{
//				wordcounter = 0;
//				if(s == "GET"){
//					linest = L_REQ;
//					m_method = GET;
//				}
//				else if(s == "HEAD"){
//					linest = L_REQ;
//					m_method = HEAD;
//				}
//				else if(s == "TRACE"){
//					linest = L_REQ;
//					m_method = TRACE;
//				}
//				else if(s == "Connection:"){
//					linest = L_CONNECTION;
//				}
//				else if(s == "User-Agent:"){
//					linest = L_AGENT;
//				}
//				else if(s == "Host:"){
//					linest = L_HOST;
//				}
//				else{
//					linest = L_UNKNOWN;
//				}
//				break;
//			}
//			case WORD_CONTENT:
//			{
//				++wordcounter;
//				switch(linest){
//					case L_REQ:
//					{
//						if(wordcounter == 1){
//							m_url = s;
//						}
//						else if(wordcounter == 2){
//							m_version = s;
//						}
//						break;
//					}
//					case L_CONNECTION:
//					{
//						if(s == "close"){
//							m_conn = CLOSE;
//						}
//						else if(s == "keep-alive"){
//							m_conn = KEEPALIVE;
//						}
//						break;
//					}
//					case L_HOST:
//					{
//						m_host += s;
//						break;
//					}
//					case L_AGENT:
//					{
//						m_agent += s;
//						m_agent += " ";
//						break;
//					}
//					case L_UNKNOWN:
//						break;
//				}
//				break;
//			}
//			case WORD_CLOSE:
//			{
//				cout << "header is read completely!" << endl;
//				break;
//			}
//		}
//
//
//
//
//    }//while
//

	int i;
	std::cin >> i;
}
