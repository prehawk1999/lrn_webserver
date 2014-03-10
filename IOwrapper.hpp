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


struct My_InOut{
    typedef char 						char_type;
    typedef io::seekable_device_tag 	category;

    My_InOut(char * Buf, std::streamsize size)
        :m_pCur(Buf),m_pBuf(Buf),m_size(size){;}

    std::streamsize read(char_type* s, std::streamsize n)
    {
       //用min来选择n还是剩余大小以防止溢出
        std::streamsize nCount = std::min(n, m_size - (m_pCur-m_pBuf));
        if(nCount)
        {
            memcpy(s,m_pCur,nCount);
            m_pCur += nCount;
            return nCount;
        }
        else{
        	return -1;
        }

    }

    std::streamsize write(const char_type* s, std::streamsize n)
    {
        std::streamsize nCount = std::min(n,m_size - (m_pCur-m_pBuf));
        if(nCount)
        {
            memcpy(m_pCur,s,nCount);
            m_pCur += nCount;
        }
        return nCount;
    }

    io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way)
    {
        // 定位
        char * pNewPos;
        switch(way)
        {
        case std::ios_base::beg:
            pNewPos = m_pBuf;
            break;
        case std::ios_base::cur:
            pNewPos = m_pCur;
            break;
        case std::ios_base::end:
            pNewPos = m_pBuf + m_size;
            break;
        }

        pNewPos += off;
        if(pNewPos < m_pBuf || pNewPos - m_pBuf > m_size)
            throw std::ios_base::failure("bad seek offset");

        m_pCur = pNewPos;
        return m_pCur - m_pBuf;
    }
private:
    char *m_pCur;
    char *m_pBuf;
    std::streamsize m_size;
};

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
		sockfd_(sockfd), pos_(0), iovc_(IOV_COUNT){}

	std::streamsize read(char_type * s, std::streamsize n){}

    std::streamsize write(const char_type * s, std::streamsize n)
    {
    	int bytes;
    	bytes = writev(sockfd_, (const iovec *)s, iovc_);

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
    struct iovec 		iov_[2];
};

#endif /* IOWRAPER_HPP_ */
