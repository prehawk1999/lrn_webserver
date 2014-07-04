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

const char * HttpRequest::s_res_httpcode[]  = {
		"100 Continue",
		"200 OK",
		"400 Bad Request",
		"404 Not Found",
		"500 Internal Error",
		"505 HTTP Version Not Supported"};

const char * HttpRequest::s_pages_addr[] = {
		"404.html",
		"index.html",
		"400.html"};

HttpRequest::HttpRequest(int connfd): sockfd_(connfd), n_bytesRecv(0)
{
	req_count 		+= 1;
	p_bufRead 		= new char[READ_BUF_SIZE];
	p_loctime   	= new char[40];
	p_fileSize 		= new char[20];
	sc_iov 		 	= new struct iovec[20];
	destroy();
}

HttpRequest::~HttpRequest(){
	req_count -= 1;
	delete [] p_loctime;
	delete [] sc_iov;
	delete [] p_bufRead;
	delete [] p_fileSize;
}


void HttpRequest::destroy(){

//	if(n_bytesRecv != 0){
//		memset(p_bufRead, '\0', n_bytesRecv);
//	}

	n_bytesSend = 0;
	n_bytesRecv = 0;
	n_newLinepos	= 0;
	st_recv = R_CLOSE;
	st_line = L_METHOD;
	st_req	= Q_REQ;

	is_keepalive = true;
	st_httpCode = _100;		// here code 100 just for not parsing empty lines.
	n_ioc = 0;				// reset the writev pointer.

	p_parse = p_bufRead;

	//if this request have map a real file, then umap it;
	if(p_fileAddr){
        munmap( p_fileAddr, sc_fileStat.st_size );
        p_fileAddr = NULL;
	}

}


// invoke by the threads in the threadpool. *MUST BE* implemented.
void HttpRequest::process(unsigned tid, bool isNew){

	is_newconn = isNew;
	if(is_newconn){
		destroy();
		cout << "***Thread no. " << tid << " get a new connection***" << endl;
	}
	else{
		cout << "***Thread no. " << tid << "processing***" << endl;
	}

	// recv sys call. app buffer is buf_read;
	ssize_t bytes = -1;
	while(bytes != 0){
		bytes = recv(sockfd_, p_bufRead + n_bytesRecv, READ_BUF_SIZE - n_bytesRecv, 0);
		if(bytes == -1){
			if(errno == EAGAIN or errno == EWOULDBLOCK){
				break;				// when recv failed, just return and doing nothing.
			}
		}
		n_bytesRecv += bytes;
	}
	cout << "<<request>>\r\n" << p_bufRead << "<<End of request>>" << endl;
	//start parsing the buffer m_read_buf;
	parseLineLoop();

	if(st_recv == R_BAD){
		cout << "invalid header!" << endl;
		fd_mod_out(m_epollfd, sockfd_); // close the peer;
		destroy();
	}
	else if(st_recv == R_CLOSE){

		//response only if you meet the \r\n at the end. AND empty request doing nothing.
		if(st_httpCode != _100){
			response();
			if(!is_keepalive){
				fd_mod_out(m_epollfd, sockfd_); // close the peer;
			}
		}// escape empty lines.
		destroy();
	}

}

//@set p_Lstart as the start of the line.
//parse every lines, set whether line is complete and which line it is.
void HttpRequest::parseLineLoop(){
	while(true){

		// new connection and new request will result in new p_endl an p_Lstart;
		if(st_recv == R_CLOSE){
			if(is_newconn){
				p_parse = p_bufRead;
			}
			else{
				p_parse = p_bufRead + n_newLinepos;
			}
		}

		p_Lstart = p_parse;

		//detect if line is invalid or it's an empty line.
		if(*p_parse == '\r' and *(p_parse+1) == '\n'){
			st_recv = R_CLOSE;
			n_newLinepos = p_parse - p_bufRead;
			return;
		}

		// move pointer p_endl to where \r\n is.
		for(; (p_parse + 2 < p_bufRead + n_bytesRecv + 1); ++p_parse){
			if( *(p_parse) == '\r' ){
				if( *(p_parse + 1) == '\n'){
					st_recv = R_END;
					parseLine();
					p_Lstart = p_parse + 2; //indicate next line.
					st_req	= Q_HEAD;
				}
				else{
					st_recv = R_BAD;
				}
			}
			else if(*(p_parse - 1) == '\r' and
					*(p_parse) == '\n' and
					*(p_parse + 1) == '\r' and
					*(p_parse + 2) == '\n'){
				st_recv = R_CLOSE;
				return;
			}
			else {
				st_recv = R_OPEN;
			}
		}

		return;

	}//infinite loop
}

// inside parseLoop(); only called when the line is completed.
void HttpRequest::parseLine(){

	getWord(p_Lstart, 1);

	st_line = L_UNKNOWN;
	for(int i = L_METHOD; i != L_UNKNOWN; ++i){
		if(strncmp(s_line_state[i], p_Wstart, p_Wend - p_Wstart - 1) == 0){
			st_line = static_cast<LINE_STATE>(i);
			break;
		}
	}

	switch(st_line){

	case L_METHOD:
	{

		//if http version is recognized, do parse the path
		if( cmpValue(s_version[0], 3) == 0 || cmpValue(s_version[1], 3) == 0 ){

			if( cmpValue("", 2) == 0 ){	//no path, bad request
				st_httpCode = _400;
				is_keepalive = false;
			}
			else{
				st_httpCode = _200;

				makeWord(p_Lstart, 2);

				//set '/' to '/index.html'
				if( cmpValue("/", 2) == 0 ){
					p_Wstart = s_pages_addr[1];
				}

				//set 404 file if file not exists or file is a dir.
				if( stat(p_Wstart, &sc_fileStat) < 0 || S_ISDIR( sc_fileStat.st_mode )){
					st_httpCode = _404;
					p_Wstart = s_pages_addr[0];
					stat(p_Wstart, &sc_fileStat);
				}

			}

		}
		else{ //if version not recognized.
			st_httpCode = _400;
			p_Wstart = s_pages_addr[2];
			stat(p_Wstart, &sc_fileStat);
			is_keepalive = false;

		}
		getFileAddr(p_Wstart);

		break;
	}
	case L_CONNECTION:
	{
		if( cmpValue(s_connection[0], 2) == 0){// get connection
			is_keepalive = false;
		}
		else{
			is_keepalive = true;
		}
		break;
	}
	case L_REFERER: //
		break;
	case L_UNKNOWN:

		//if request line is unrecognizable, then it's a bad request.
		if(st_req == Q_REQ){
			st_httpCode = _400;
			p_Wstart = s_pages_addr[2];
			stat(p_Wstart, &sc_fileStat);
			getFileAddr(p_Wstart);
		}
		break;
	default:
		break;
	}



}


//map the file
void HttpRequest::getFileAddr(char * filename){
	int filed = open( filename, O_RDONLY );
	p_fileAddr = ( char* )mmap( 0, sc_fileStat.st_size, PROT_READ, MAP_PRIVATE, filed, 0 );
	close( filed );
}

//generate current Date string.
char * HttpRequest::genDate(){
	time_t rawtime = time(NULL);
	ctime_r(&rawtime, p_loctime);
	return p_loctime;
}

//get words and compare the value with specific string.
int HttpRequest::cmpValue(const char * value, int count){
	getWord(p_Lstart, count);
	return strncmp(p_Wstart, value, p_Wend - p_Wstart);
}

//get word number %count% and set the char after the word as '\0'
void HttpRequest::makeWord(char * pos, int count){
	getWord(pos, count);
	*p_Wend = '\0';
}

//@set w_startPos, w_endPos
//move %count% words
void HttpRequest::getWord(char * pos, int count){
	for(int i=0; i != count; ++i){
		p_Wstart = pos;
		for(; *pos != 0x20 and *pos != 0x0 and *pos != 0xd; ++pos);

		if(*pos == 0x0){
			p_Wend = pos - 2 ; // skip normal chars.
		}
		else{
			p_Wend = pos;
		}

		for(; *pos == 0x20 and *pos != 0x0; ++pos); // skip spaces.
	}
}

//add header.
void HttpRequest::addData(const char * head, size_t size){
	sc_iov[n_ioc].iov_base = head ;
	if(!size){
		sc_iov[n_ioc].iov_len	= strlen(head);
		n_bytesSend += strlen(head);
		std::cerr << head;
	}
	else{
		sc_iov[n_ioc].iov_len  = size;
		n_bytesSend += size;
		std::cerr << "total bytes to send: " << n_bytesSend;
	}
	n_ioc += 1;
}


//invoke by the main thread, that is WebServer .*MUST BE* implemented.
void HttpRequest::response(){



	//line 1: HTTP/1.1 200 OK
	addData(s_version[0]);
	addData(s_res_httpcode[st_httpCode]);
	addData("\r\n");

	//line 2: Date: dsfsdfsdfoieworfew
	addData(s_line_state[L_DATE]);
	addData(genDate()); //it contains \r\n inside the date string.

	//line 3: Connection: keep-alive;
	addData(s_line_state[L_CONNECTION]);
	addData(s_connection[is_keepalive]);
	addData("\r\n");

	//line final: file
	if(p_fileAddr){
		sprintf(p_fileSize, "%ld", sc_fileStat.st_size);
		addData(s_line_state[L_CONLEN]);
		addData(p_fileSize);
		addData("\r\n");

		addData("Content-Type: ");
		addData("text/html");			//todo: classify types.
		addData("\r\n");

		addData("\r\n");
		addData(p_fileAddr, sc_fileStat.st_size);
	}


	int bytes;
	while(true){
		bytes = writev(sockfd_, sc_iov, n_ioc);
		assert(bytes != -1);
		n_bytesSend -= bytes;
		if(n_bytesSend == 0){
			break;
		}
	}

	cout << "**response no. " << req_count << "**" << endl;
}

