/*
 * http_parser.h
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */

#ifndef HTTP_PARSER_H_
#define HTTP_PARSER_H_

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>

class HttpParser
{
public:
	enum REQ_STATE{REQ_OK, REQ_WAIT, REQ_BAD};

private:
	enum LINE_STATE{LINE_OK, LINE_WAIT, LINE_BAD};



public:
	HttpParser();
	REQ_STATE parse(char * reqbuffer);


public:
	std::string res_header;
	std::string res_content;

};



#endif /* HTTP_PARSER_H_ */
