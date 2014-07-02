/*
 * http_parser.h
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */

#ifndef HTTP_PARSER_H_
#define HTTP_PARSER_H_

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
	enum HTTP_STATUS{_100, _200, _400, _403, _404, _500};

	enum RECV_STATE{R_OPEN, R_BAD, R_END, R_CLOSE};
	enum LINE_STATE{L_METHOD, L_HOST, L_UA, L_ACCEPT,
		L_REFERER, L_CONNECTION, L_CONLEN, L_SERVER, L_UNKNOWN};

	static const char * s_line_state[];
	static const char * s_res_code[];
	static const char * s_version[];
	static const char * s_connection[];

public:
	HttpRequest(int connfd);
	HttpRequest(HttpRequest * const & h);
	~HttpRequest();

	void process(unsigned tid, bool isNew); //call iowrapper to read a complete line, then fill members.
	void response();
	void destroy();

private:
	void clearStates();

	void parse();
	void parse2();
	void switchLine();
	void initStates();
	void setHttpState();

	void post();
	void writeResponse(stringstream & ss, char * file_addr, ssize_t file_size);
	void writeHeader();
	void setFileStat();


	int parseLineLoop( );
	void parseLine( );
	void getWord(char * p, int count);

	void getValue(char * & value, int count);
	void getConnection();

private:
	//internal socket descritor
	int							sockfd_;

	//receive handler
	io::stream<RecvHandler> 	in_;

	//send handler
	io::stream<SendHandler>		out_;

	//HTTP request newline flag
	bool						isNewLine_;

	//state of current looping word
    WORD_STATE 					wordst_;

    //state of line
    RECV_STATE					linest_;

    //current word
    string						word_;

    //word no. in one line
    int							wordcount_;

    //HTTP response header
    stringstream				response_;

private:

    //buffer fields
    char * 			m_read_buf;
    char *			m_write_buf;

	//request fields
	METHOD			m_method;
	string			m_url;
	string			m_version;

	string			m_agent;

	string			m_host;
	CONNECTION		m_conn;

	//response fields
	HTTP_STATUS		m_status;
	string			m_contenttype;
	string			m_url_file;



	////////////////////////////////////
	// GAP
	///////////////////////////////////



	//request test
	static int		req_count;
	int 			test_count;

	//request state
	ssize_t 			bytes_recv;
	ssize_t				bytes_;
	ssize_t				n_linePos; // new request

	char *				P;		//parse pivot
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
	char *				fileAddr;
	char * 				clientConn;

	char *				m_file_addr;
	struct stat			m_file_stat;
	struct iovec  		m_iov[2];


};



#endif /* HTTP_PARSER_H_ */
