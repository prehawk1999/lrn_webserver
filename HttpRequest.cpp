/*
 * http_parser.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */
#include "HttpRequest.h"


int HttpRequest::req_count = 0;

const char * HttpRequest::s_version[] = {"HTTP/1.0","HTTP/1.1"};
const char * HttpRequest::s_connection[] = {"close", "keep-alive"};

const char * HttpRequest::s_line_state[] = {
		"GET",
		"Host:",
		"User-Agent:",
		"Accept:",
		"Referer:",
		"Connection:",
		"Content-Length:",
		"Server:"};

const char * HttpRequest::s_res_code[]  = {
		"200 OK",
		"400 Bad Request",
		"404 Not Found",
		"500 Internal Error"};

HttpRequest::HttpRequest(int connfd):
	 sockfd_(connfd),
	 in_(connfd),
	 out_(connfd),
	 isNewLine_(true),
	 wordst_(W_TAG),
	 word_(),
	 wordcount_(0),
	 response_(),
	 bytes_recv(-1),
	 bytes_(0)
{
	m_read_buf = new char[READ_BUF_SIZE];
	req_count += 1;
	test_count = 0;
	destroy();
}

HttpRequest::HttpRequest(HttpRequest * const & h):
	 sockfd_(h->sockfd_),
	 in_(h->sockfd_),
	 out_(h->sockfd_),
	 isNewLine_(h->isNewLine_),
	 wordst_(h->wordst_),
	 word_(),
	 wordcount_(0),
	 response_(),
	 bytes_recv(-1),
	 bytes_(0)
{
	m_read_buf = new char[READ_BUF_SIZE];
	req_count += 1;
	test_count = 0;
	destroy();
}

HttpRequest::~HttpRequest(){
	req_count -= 1;
}

void HttpRequest::clearStates(){

	//request
	m_method = GET;
	m_url = "";

	m_agent = "";

	m_host = "";
	m_conn = CLOSE;

	//response
	m_status = _100;
	m_contenttype = "";
	m_url_file = "";
}

void HttpRequest::destroy(){
	RECV_ = R_CLOSE;
	LINE_ = L_METHOD;
	ISKEEPALIVE_ = true;
}


// invoke by the threads in the threadpool. *MUST BE* implemented.
void HttpRequest::process(unsigned tid, bool isNew){

	ISNEWCONN_ = isNew;
	if(ISNEWCONN_){
		destroy();
		RECV_ = R_CLOSE;
		cout << "**Thread no. " << tid << " get a new connection**" << endl;
	}
	else{
		cout << "**Thread no. " << tid << "**" << endl;
		test_count += 1;
	}

	// recv sys call. app buffer is m_read_buf;
	while(bytes_recv != 0){
		bytes_recv = recv(sockfd_, m_read_buf + bytes_, READ_BUF_SIZE - bytes_, 0);
		if(bytes_recv == -1){
			if(errno == EAGAIN or errno == EWOULDBLOCK){
				break;
			}
		}
		bytes_ += bytes_recv;
	}

	//start parsing the buffer m_read_buf;
	while( parseLineLoop() ){}

	if(RECV_ == R_BAD){
		cout << "invalid header!" << endl;
	}
	else if(RECV_ == R_CLOSE){
		// do nothing
		if(!ISKEEPALIVE_){
			fd_mod_out(m_epollfd, sockfd_); // close the peer;
		}
		else{

		}
	}
	else{
		//response();
		cout << "**response no. " << req_count << "**" << endl;
	}


}

//@return: -1 error, 1 not complete, should be invoked again. 0 end of reading.
//parse every lines, set whether line is complete and which line it is.
int HttpRequest::parseLineLoop(){

	// new connection and new request will result in new E an l_startPos;
	if(RECV_ == R_CLOSE){
		if(ISNEWCONN_){
			E = m_read_buf;
		}
		else{
			E = m_read_buf + n_linePos;
		}
		l_startPos = E;
	}
	else if(RECV_  == R_END){
		E += 3;
		l_startPos = E;
	}

	//detect if line is invalid or it's an empty line.
	if(E + 2 > m_read_buf + bytes_){
		RECV_ = R_BAD;
		return -1;
	}
	else if(*E == '\r' and *(E+1) == '\n'){
		RECV_ = R_CLOSE;
		n_linePos = E + 2 - m_read_buf;
		return 0;
	}

	// move pointer E to where \r\n is.
	for(;E + 2 < m_read_buf + bytes_; ++E){
		if( *(E + 1) == '\r' ){
			if( *(E + 2) == '\n'){
				RECV_ = R_END;
				l_endPos = E;
				parseLine();
				break;
			}
			else{
				RECV_ = R_BAD;
				return -1;
			}
		}
		else {
			RECV_ = R_OPEN;
		}
	}

	if(E + 3 == m_read_buf + bytes_)
		return 0; // end of reading.
	else
		return 1;

}

// inside parseLoop(); only called when the line is completed.
void HttpRequest::parseLine(){

	P = l_startPos;

	getWord(P, 1);
	for(int i = L_METHOD; i != L_UNKNOWN; ++i){
		if(strncmp(s_line_state[i], w_startPos, w_endPos - w_startPos) == 0){
			LINE_ = static_cast<LINE_STATE>(i);
			break;
		}
		LINE_ = L_UNKNOWN;
	}

	switch(LINE_ ){

	case L_METHOD:
		getValue(fileAddr, 2);
		break;
	case L_CONNECTION:
		getValue(clientConn, 2);
		if(strncmp(s_connection[0], clientConn, w_endPos - w_startPos) == 0){// get connection
			ISKEEPALIVE_ = false;
		}
		else{
			ISKEEPALIVE_ = true;
		}
		break;
	default:
		break;
	}

}

//get words into value
void HttpRequest::getValue(char * & value, int count){
	getWord(l_startPos, count);
	value = new char[w_endPos - w_startPos];
	strncpy(value, w_startPos, w_endPos - w_startPos);
}


//@set w_startPos, w_endPos
//move %count% words
void HttpRequest::getWord(char * p, int count){
	for(int i=0; i != count; ++i){
		w_startPos = p;
		for(; *p != 0x20 and *p != 0x0; ++p);

		if(*p == 0x0){
			w_endPos = p - 2 ; // skip normal chars.
		}
		else{
			w_endPos = p;
		}

		for(; *p == 0x20 and *p != 0x0; ++p); // skip spaces.
	}
}


//invoke by the main thread, that is WebServer .*MUST BE* implemented.
void HttpRequest::response(){



	cout << "**response has been sent.**" << endl;
}


///////////////////////////////////////////////
//					GAP
///////////////////////////////////////////////


void HttpRequest::switchLine(){

}

void HttpRequest::initStates(){

}

void HttpRequest::setHttpState(){

	switch(wordst_){
		case W_TAG:
		{
			switchLine();
			break;
		}
		case W_CONTENT:
		{
			initStates();
			break;
		}
		case W_BAD:
		{
			cout << "header is incomplete!" << endl;
			break;
		}
	}
}




void HttpRequest::parse(){
	bool CRLF = true;
    while(in_ >> word_){
    	CRLF = false;
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
    			if(!isNewLine_){
					cout << word_ << endl;
					wordst_ = W_CONTENT;
					isNewLine_ = true;
    			}
    			else{
    				cout << "Incomplete header!" << endl;
    			}
    			break;
    		case -1: // eof
    			wordst_ = W_BAD;
    			in_.clear();
    			in_.seekg(npos - len, std::ios_base::beg);		//read this word again.
    			break;
    		default:
    			cout << "something bad happened" << endl;
    			break;

    	}//switch
    	setHttpState();
    }
    in_.clear();
    in_.seekg(0, std::ios_base::beg);
    if(m_version != "HTTP/1.1" and m_version  != "HTTP/1.0" ){
    	m_url_file = "";
    	cout << "Incomplete header!" << endl;
    }
    cout << endl;
}

void HttpRequest::post(){
	//cout << "one post" << endl;

	setFileStat();
	stringstream res;
	res << m_version << " ";
	switch(m_status){
	case _100:
		res << 100 << " Continue\r\n";
		break;
	case _200:
		res << 200 << " OK\r\n";
		break;
	case _400:
		res << 400 << " Bad Request\r\n";
		break;
	case _403:
		res << 403 << " Forbidden\r\n";
		break;
	case _404:
		res << 404 << " Not Found\r\n";
		break;
	case _500:
		res << 500 << " Internal Error\r\n";
		break;
	}

	res << "Content-Length: " << m_file_stat.st_size << "\r\n";

	res << "Connection: ";
	if(m_conn == CLOSE){
		res << "close\r\n";
	}
	else{
		res << "keep-alive\r\n" << "Keep-Alive: " << 86400 << "\r\n";
	}

	res << "\r\n";

	//res is the response head, the others are file bits.
	writeResponse(res, m_file_addr, m_file_stat.st_size);

    if( m_file_addr )
    {
        munmap( m_file_addr, m_file_stat.st_size );
        m_file_addr = NULL;
    }
}


void HttpRequest::setFileStat(){

	if(m_url_file != ""){
		if( stat(m_url_file.c_str(), &m_file_stat) < 0){
			m_status = _404;
			m_url_file = page_404;
			stat(m_url_file.c_str(), &m_file_stat);
		}
		else{
			m_status = _200;
		}

		if( ! ( m_file_stat.st_mode & S_IROTH ) ){
			m_status = _403;
			//return;
		}

//		if(S_ISDIR( m_file_stat.st_mode )){
//			m_status  = _400;
//			return;
//		}

	    int fd = open( m_url_file.c_str(), O_RDONLY );
	    m_file_addr = ( char* )mmap( 0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );
	    close( fd );
	}
	else{
		m_status = _500;
	}
}

void HttpRequest::writeResponse(stringstream & ss, char * file_addr, ssize_t file_size){
	
	char strRes[256];
	memset(strRes, '\0', 256);
	ss.getline(strRes, 100, '\0');
	ssize_t ttLen = 0, reslen = strlen(strRes);

	m_iov[0].iov_base = strRes;
	m_iov[0].iov_len  = reslen;

	m_iov[1].iov_base = m_file_addr;
	m_iov[1].iov_len = m_file_stat.st_size;

	ttLen = reslen + m_file_stat.st_size;
	
	int ret;
	while(true){
		ret = writev(sockfd_, m_iov, 2);
		assert(ret != -1);
		ttLen -= ret;
		if(ttLen == 0)
			break;
	}
}
