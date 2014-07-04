/*
 * http_parser.h
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */

#ifndef HTTP_PARSER_H_
#define HTTP_PARSER_H_

#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <string>
#include <boost/logic/tribool.hpp>

#include "Request.h"
#include "utils.h"
#include "config.hpp"
#include "IOwrapper.hpp"


using boost::logic::tribool;
using boost::indeterminate;
using std::string;
using std::cout;
using std::endl;
using std::istringstream;
using std::ostringstream;
using std::stringstream;

const string home_dir(HOME_DIR);

const string page_404(PAGE_404);


class HttpRequest: public Request
{

	//GAP
	enum HTTP_STATUS{_100, _200, _400, _404, _500, _505};
	enum RECV_STATE{R_OPEN, R_BAD, R_END, R_CLOSE};
	enum REQ_STATE{Q_REQ, Q_HEAD, Q_BODY};
	enum LINE_STATE{L_METHOD, L_HOST, L_UA, L_ACCEPT,
		L_REFERER, L_CONNECTION, L_CONLEN, L_DATE, L_UNKNOWN};

	static const char * s_version[];
	static const char * s_connection[];
	static const char * s_line_state[];
	static const char * s_res_httpcode[];
	static const char * s_pages_addr[];

public:
	HttpRequest(int connfd);
	HttpRequest(HttpRequest * const & h);
	~HttpRequest();

	void process(unsigned tid, bool isNew); //call iowrapper to read a complete line, then fill members.
	void response();
	void destroy();

private:

	void writeResponse(stringstream & ss, char * file_addr, ssize_t file_size);

	void parseLineLoop( );
	void parseLine( );
	void getWord(char * p, int count);
	void getFileAddr(char * filename);
	void addData(const char * head, size_t size=0);
	char * genDate();
	int cmpValue(const char * value, int count);
	void makeWord(char * pos, int count);
	void getConnection();

private:
	//internal socket descritor
	int							sockfd_;


private:


    //buffer fields
    char * 			p_bufRead;				// should memset when new connection

	//request test
	static int		req_count;

	//request state
	ssize_t 			n_bytesRecv;
	ssize_t				n_bytesSend;
	ssize_t				n_newLinepos; // new request

	char *				p_parse;		//read pivot

	char * 				p_Lstart;
	char *				p_Lend;
	char * 				p_Wstart;
	char * 				p_Wend;


	RECV_STATE			st_recv;
	LINE_STATE			st_line;
	REQ_STATE			st_req;

	bool				is_newconn;
	bool				is_newline;
	bool				is_keepalive;


	//response field
	HTTP_STATUS			st_httpCode;

	//file location, a path
	char *				p_fileName;

	//file pointer that point to the beginning of the file.
	char *				p_fileAddr;

	//file status.
	struct stat			sc_fileStat;
	char *				p_fileSize;


	struct iovec * 		sc_iov;
	int					n_ioc;

	char *				p_loctime;


};



#endif /* HTTP_PARSER_H_ */
