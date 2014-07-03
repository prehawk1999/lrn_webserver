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
	enum REQ_STATE{REQ_OK, REQ_WAIT, REQ_BAD};
	enum WORD_STATE{W_TAG, W_CONTENT, W_BAD };
	enum METHOD{GET, HEAD, TRACE};
	enum CONNECTION{KEEPALIVE, CLOSE};

	//GAP
	enum HTTP_STATUS{_100, _200, _400, _404, _500, _505};
	enum RECV_STATE{R_OPEN, R_BAD, R_END, R_CLOSE};
	enum LINE_STATE{L_METHOD, L_HOST, L_UA, L_ACCEPT,
		L_REFERER, L_CONNECTION, L_CONLEN, L_DATE, L_UNKNOWN};

	static const char * s_version[];
	static const char * s_connection[];
	static const char * s_line_state[];
	static const char * s_res_code[];
	static const char * s_pages_addr[];

public:
	HttpRequest(int connfd);
	HttpRequest(HttpRequest * const & h);
	~HttpRequest();

	void process(unsigned tid, bool isNew); //call iowrapper to read a complete line, then fill members.
	void response();
	void destroy();

private:


	//GAP
	void writeResponse(stringstream & ss, char * file_addr, ssize_t file_size);

	int parseLineLoop( );
	void parseLine( );
	void getWord(char * p, int count);

	void addData(const char * head, size_t size=0);
	char * genDate();
	int cmpValue(const char * value, int count);
	void getValue(char * & value, int count);
	void getConnection();

private:
	//internal socket descritor
	int							sockfd_;


private:

	////////////////////////////////////
	// GAP
	///////////////////////////////////

    //buffer fields
    char * 			m_read_buf;

	//request test
	static int		req_count;
	int 			test_count;

	//request state
	ssize_t 			bytes_recv;
	ssize_t				bytes_send;
	ssize_t				n_linePos; // new request

	char *				E;		//read pivot

	char * 				l_startPos;
	char *				l_endPos;
	char * 				w_startPos;
	char * 				w_endPos;


	RECV_STATE			RECV_;
	LINE_STATE			LINE_;

	bool				ISNEWCONN_;
	bool				ISNEWLINE_;
	bool				ISKEEPALIVE_;


	//response field
	HTTP_STATUS			RET_CODE_;

	//file location, a path
	char *				fileName;

	//file pointer that point to the beginning of the file.
	char *				fileAddr;

	//file status.
	struct stat			fileStat;
	char *				fileSize;


	struct iovec * 		IOV_;
	int					IOC_;

	char *				loc_time;


};



#endif /* HTTP_PARSER_H_ */
