/*
 * threadpool.hpp
 *
 *  Created on: Feb 14, 2014
 *      Author: prehawk
 */

#ifndef THREADPOOL_HPP_
#define THREADPOOL_HPP_

#include <pthread.h>
#include <boost/progress.hpp>
#include "locker.hpp"

template< typename T>
class threadpool
{
public:
	threadpool(int threadnum);
	~threadpool();
	void * run( void * arg );
private:
	int m_threadnum;
	pthread_t * pth_array;
	bool m_stop;

};

template<typename T>
threadpool<T>::threadpool(int threadnum):
	m_threadnum(threadnum), m_stop(false)
{
	pth_array = new pthread_t[threadnum];
}

template<typename T>
void * threadpool<T>::run(void * arg)
{

}

#endif /* THREADPOOL_HPP_ */
