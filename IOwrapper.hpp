/*
 * IOwraper.hpp
 *
 *  Created on: Mar 8, 2014
 *      Author: prehawk
 */

#ifndef IOWRAPER_HPP_
#define IOWRAPER_HPP_

#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/positioning.hpp>
#include <time.h>

#include "config.hpp"



namespace io = boost::iostreams;


struct RecvHandler{
	typedef char 						char_type;
	typedef io::seekable_device_tag 	category;

	RecvHandler(int sockfd):
		sockfd_(sockfd), pos_(0){}

	std::streamsize read(char_type * s, std::streamsize n)
	{
		int bytes;
		bytes = recv(sockfd_, s + pos_, n - pos_, 0);
		if(bytes == -1){
			return -1;
		}
		else{
			pos_ += bytes;
			return bytes;
		}
	}

    std::streamsize write(const char_type* s, std::streamsize n){ }

	std::streampos seek(io::stream_offset off, std::ios_base::seekdir way )
	{
		// 定位
		int pNewPos;
		switch(way)
		{
		case std::ios_base::beg:
			pNewPos = 0;
			break;
		case std::ios_base::cur:
			pNewPos = pos_;
			break;
		case std::ios_base::end: // should not be used
			pNewPos = MAX_BUF;
			break;
		}

		pNewPos += off;
		if(pNewPos > MAX_BUF)
			throw std::ios_base::failure("bad seek offset");

		pos_ = pNewPos;
		return pos_;
	}

private:
	int 				sockfd_;
	std::streamsize 	pos_;
};

struct SendHandler{
	typedef char 						char_type;
	typedef io::seekable_device_tag 	category;

	SendHandler(int sockfd):
		sockfd_(sockfd), pos_(0), iovc_(0){}

	std::streamsize read(char_type * s, std::streamsize n){}

    std::streamsize write(const char_type * s, std::streamsize n)
    {
    	// "cout << 0;"  to indicate the end of input, and send the iovec.
    	if( *(s + pos_) == 0x30 ){
    		int bytes;
    		bytes = writev(sockfd_, iov_, iovc_);
    		return bytes;
    	}
    	else{
    		iov_[iovc_++] = *( (const iovec *)(s + pos_) );
    		pos_ += sizeof(iovec);
    		return 0;
    	}


    }

	std::streampos seek(io::stream_offset off, std::ios_base::seekdir way)
	{
        // 定位
        int pNewPos;
        switch(way)
        {
        case std::ios_base::beg:
            pNewPos = 0;
            break;
        case std::ios_base::cur:
            pNewPos = pos_;
            break;
        case std::ios_base::end: // should not be used
            pNewPos = MAX_BUF;
            break;
        }

        pNewPos += off;
        if(pNewPos < pos_ || pNewPos > MAX_BUF)
            throw std::ios_base::failure("bad seek offset");

        pos_ = pNewPos;
        return pos_;
	}

private:
	int 				sockfd_;
	std::streamsize 	pos_;
	int					iovc_;
	char *				test[10];
    struct iovec  		iov_[2];

};

#endif /* IOWRAPER_HPP_ */
