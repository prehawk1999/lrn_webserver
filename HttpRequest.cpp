/*
 * http_parser.cpp
 *
 *  Created on: Feb 27, 2014
 *      Author: prehawk
 */
#include "HttpRequest.h"


int HttpRequest::req_count = 0;

const char * HttpRequest::s_version[] = {"HTTP/1.0 ","HTTP/1.1 "};
const char * HttpRequest::s_connection[] = {"close ", "keep-alive "};

const char * HttpRequest::s_line_state[] = {
		"GET ",
		"Host: ",
		"User-Agent: ",
		"Accept: ",
		"Referer: ",
		"Connection: ",
		"Content-Length: ",
		"Date: "};

const char * HttpRequest::s_res_code[]  = {
		"100 Continue",
		"200 OK",
		"400 Bad Request",
		"404 Not Found",
		"500 Internal Error",
		"505 HTTP Version Not Supported"};

const char * HttpRequest::s_pages_addr[] = {
		"404.html",
		"index.html"};

HttpRequest::HttpRequest(int connfd): sockfd_(connfd)
{
	m_read_buf 		= new char[READ_BUF_SIZE];
	loc_time   		= new char[40];
	IOV_ 		 	= new struct iovec[20];
	fileSize 		= new char[10];
	req_count 		+= 1;
	destroy();
}

HttpRequest::~HttpRequest(){
	req_count -= 1;
	delete [] loc_time;
	delete [] IOV_;
	delete [] m_read_buf;
	delete [] fileSize;
}


void HttpRequest::destroy(){

	if(bytes_recv != -1){
		memset(m_read_buf, '\0', bytes_recv);
	}

	bytes_send = 0;
	bytes_recv = -1;
	RECV_ = R_CLOSE;
	LINE_ = L_METHOD;
	ISKEEPALIVE_ = true;
	RET_CODE_ = _100;

	E = m_read_buf;
	//if this request have map a real file, then umap it;
	if(fileAddr){
        munmap( fileAddr, fileStat.st_size );
        fileAddr = NULL;
	}

	//set the return code to 100, just for not parsing empty lines.
	IOC_ = 0;
}


// invoke by the threads in the threadpool. *MUST BE* implemented.
void HttpRequest::process(unsigned tid, bool isNew){

	ISNEWCONN_ = isNew;
	if(ISNEWCONN_){
		destroy();
		cout << "**Thread no. " << tid << " get a new connection**" << endl;
	}
	else{
		cout << "**Thread no. " << tid << "**" << endl;
	}

	// recv sys call. app buffer is m_read_buf;
	ssize_t bytes = -1;
	while(bytes != 0){
		bytes = recv(sockfd_, m_read_buf + bytes_recv, READ_BUF_SIZE - bytes_recv, 0);
		if(bytes == -1){
			if(errno == EAGAIN or errno == EWOULDBLOCK){
				break;
			}
		}
		bytes_recv += bytes;
	}

	//start parsing the buffer m_read_buf;
	parseLineLoop();

	if(RECV_ == R_BAD){
		cout << "invalid header!" << endl;
		fd_mod_out(m_epollfd, sockfd_); // close the peer;
		RECV_ = R_CLOSE;
	}
	else if(RECV_ == R_CLOSE){

		//response only if you meet the \r\n at the end. AND empty request doing nothing.
		if(RET_CODE_ != _100){
			response();
			if(!ISKEEPALIVE_){
				fd_mod_out(m_epollfd, sockfd_); // close the peer;
			}
		}// escape empty lines.
		destroy();
	}

}

//@return: -1 error, 1 not complete, should be invoked again. 0 end of reading.
//@set l_startPos as the start of the line, and l_endPos as the end of the line.
//parse every lines, set whether line is complete and which line it is.
int HttpRequest::parseLineLoop(){

	while(true){

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
		if(*E == '\r' and *(E+1) == '\n'){
			RECV_ = R_CLOSE;
			n_linePos = E + 2 - m_read_buf;
			break;
		}

		//todo: infinite loop bug!
		// move pointer E to where \r\n is.
		for(;E + 2 < m_read_buf + bytes_offset; ++E){
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

		if(E + 3 == m_read_buf + bytes_offset)
			break; // end of reading.
	}
	return 0;
}

// inside parseLoop(); only called when the line is completed.
void HttpRequest::parseLine(){

	getWord(l_startPos, 1);
	for(int i = L_METHOD; i != L_UNKNOWN; ++i){
		if(strncmp(s_line_state[i], w_startPos, w_endPos - w_startPos - 1) == 0){
			LINE_ = static_cast<LINE_STATE>(i);
			break;
		}
		LINE_ = L_UNKNOWN;
	}

	switch(LINE_ ){

	case L_METHOD:

		//if http version is recognized, do parse the path
		if( cmpValue(s_version[0], 3) == 0 || cmpValue(s_version[1], 3) == 0 ){
			getValue(fileName, 2);

			if( strcmp(fileName, "") != 0 ){

				//set '/' to '/index.html'
				if( strcmp(fileName, "/") == 0 ){
					fileName = s_pages_addr[1];
				}

				//set 404 file if file not exists or file is a dir.
				if( stat(fileName, &fileStat) < 0 || S_ISDIR( fileStat.st_mode )){
					RET_CODE_ = _404;
					fileName = s_pages_addr[0];
					stat(fileName, &fileStat);
				}
				else{
					RET_CODE_ = _200;
				}

			    int fd = open( fileName, O_RDONLY );
			    fileAddr = ( char* )mmap( 0, fileStat.st_size, PROT_READ, MAP_PRIVATE, fd, 0 );
			    close( fd );
			}
			else{
				RET_CODE_ = _400;
				ISKEEPALIVE_ = false;
			}

		}
		else{ //if version not recognized.
			RET_CODE_ = _400;
			ISKEEPALIVE_ = false;
		}
		break;
	case L_CONNECTION:
		char * clientConn;
		getValue(clientConn, 2);
		if(strncmp(s_connection[0], clientConn, w_endPos - w_startPos) == 0){// get connection
			ISKEEPALIVE_ = false;
		}
		else{
			ISKEEPALIVE_ = true;
		}
		break;
	case L_REFERER:

		break;
	default:
		//E = m_read_buf;
		break;
	}

}

//generate current Date string.
char * HttpRequest::genDate(){
	time_t rawtime = time(NULL);
	ctime_r(&rawtime, loc_time);
	return loc_time;
}

//get words and compare the value with specific string.
int HttpRequest::cmpValue(const char * value, int count){
	getWord(l_startPos, count);
	int ret = strncmp(w_startPos, value, strlen(value) - 1);
	return ret;
}

//get words into value
void HttpRequest::getValue(char * & value, int count){
	getWord(l_startPos, count);
	value = new char[w_endPos - w_startPos];
	strncpy(value, w_startPos, w_endPos - w_startPos);
}

//@set w_startPos, w_endPos
//move %count% words
void HttpRequest::getWord(char * pos, int count){
	for(int i=0; i != count; ++i){
		w_startPos = pos;
		for(; *pos != 0x20 and *pos != 0x0; ++pos);

		if(*pos == 0x0){
			w_endPos = pos - 2 ; // skip normal chars.
		}
		else{
			w_endPos = pos;
		}

		for(; *pos == 0x20 and *pos != 0x0; ++pos); // skip spaces.
	}
}

//add header.
void HttpRequest::addData(const char * head, size_t size){
	IOV_[IOC_].iov_base = head ;
	if(!size){
		IOV_[IOC_].iov_len	= strlen(head);
		bytes_send += strlen(head);
		std::cerr << head;
	}
	else{
		IOV_[IOC_].iov_len  = size;
		bytes_send += size;
	}
	IOC_ += 1;
}


//invoke by the main thread, that is WebServer .*MUST BE* implemented.
void HttpRequest::response(){



	//line 1: HTTP/1.1 200 OK
	addData(s_version[0]);
	addData(s_res_code[RET_CODE_]);
	addData("\r\n");

	// todo: multipul threads problem.
	//line 2: Date: dsfsdfsdfoieworfew
//	addData(s_line_state[L_DATE]);
//	addData(genDate()); //it contains \r\n inside the date string.

	//line 3: Connection: keep-alive;
	addData(s_line_state[L_CONNECTION]);
	addData(s_connection[ISKEEPALIVE_]);
	addData("\r\n");

	//line final: file
	if(fileAddr){
		sprintf(fileSize, "%ld", fileStat.st_size);
		addData(s_line_state[L_CONLEN]);
		addData(fileSize);
		addData("\r\n");

		addData("Content-Type: ");
		addData("text/html");			//todo: classify types.
		addData("\r\n");

		addData("\r\n");
		addData(fileAddr, fileStat.st_size);
	}

	//line close
	addData("\r\n");

	int ret;
	while(true){
		ret = writev(sockfd_, IOV_, IOC_);
		assert(ret != -1);
		bytes_send -= ret;
		if(bytes_send == 0){

			break;
		}
	}


	cout << "**response no. " << req_count << "**" << endl;
}

///////////////////////////////////////////////
//					GAP
///////////////////////////////////////////////


