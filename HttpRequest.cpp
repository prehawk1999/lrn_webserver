/*
 * http_parser.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */
#include "HttpRequest.h"


HttpRequest::HttpRequest(int connfd):
	 sockfd_(connfd),
	 in_(connfd),
	 out_(connfd),
	 isNewLine_(true),
	 wordst_(W_TAG),
	 linest_(L_REQ),
	 word_(),
	 wordcount_(0),
	 response_()
{
	fd_add(m_epollfd, sockfd_);

}

HttpRequest::HttpRequest(HttpRequest * const & h):
	 sockfd_(h->sockfd_),
	 in_(h->sockfd_),
	 out_(h->sockfd_),
	 isNewLine_(h->isNewLine_),
	 wordst_(h->wordst_),
	 linest_(h->linest_),
	 word_(),
	 wordcount_(0),
	 response_()
{
	fd_add(m_epollfd, sockfd_);
}

HttpRequest::~HttpRequest(){}

void HttpRequest::clearStates(){

	//request
	m_method = GET;
	m_url = "";
	m_version = "HTTP1.1";

	m_agent = "";

	m_host = "";
	m_conn = CLOSE;

	//response
	m_status = _100;
	m_contenttype = "";
	m_url_file = "";
}

void HttpRequest::destroy(){
}

// invoke by the threads in the threadpool. *MUST BE* implemented.
void HttpRequest::process(int tid){

	cout << "**Thread no. " << tid  << " is processing.**" << endl;
	parse();
	fd_mod_out(m_epollfd, sockfd_);		// tag input to be avaliable.
}


//invoke by the main thread, that is WebServer .*MUST BE* implemented.
void HttpRequest::response(){

	post();
	cout << "**response has been sent.**" << endl;
	fd_mod_in(m_epollfd, sockfd_);		// tag output to be avaliable.
}

void HttpRequest::switchLine(){
	wordcount_ = 0;
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

void HttpRequest::initStates(){

	++wordcount_;
	switch(linest_){
		case L_REQ:
		{
			if(wordcount_ == 1){
				m_url = word_;
				if(m_url != "/"){
					m_url_file = home_dir + m_url;
				}
				else{
					m_url_file = home_dir + "/index";
				}
			}
			else if(wordcount_ == 2){
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
			return;
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
	ssize_t reslen = strlen(strRes);

	out_.flags(std::ios_base::unitbuf);
	m_iov.iov_base = strRes;
	m_iov.iov_len  = reslen;
	out_.write((const char *)&m_iov, sizeof m_iov);

	m_iov.iov_base = m_file_addr;
	m_iov.iov_len = m_file_stat.st_size;
	out_.write((const char *)&m_iov, sizeof m_iov);

	out_ << 0;
	out_.clear();
	out_.seekp(0, std::ios_base::beg);
}
