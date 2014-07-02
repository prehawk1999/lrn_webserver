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
#include <deque>
#include <list>
#include <iostream>
#include <exception>


#include "locker.hpp"

template< typename T>
class threadpool
{
public:
	typedef std::list< std::pair<T, bool> >							reql_t;

	threadpool(int threadnum = 1, size_t queuelen = 10000);
	~threadpool();
	bool append(T request, bool isNew);
	void run(){m_worklocker.unlock();}
	int queuesize(){return m_req_list.size();}
	int queuecap(){return m_max_reqnum;}
	int poolsize(){return m_threadnum;}
private:
	void dowork();
	static void * worker(void * arg);

private:
	int 			m_threadnum;
	size_t 			m_max_reqnum;

private:
	sem 			m_queuestat;		//seems that it's used to prevent dead lock
	locker 			m_queuelocker;
	locker 			m_worklocker;
	pthread_t * 	pth_array;			//pthread_create need native pointer, boost::shared_ptr is not avail
	reql_t 			m_req_list;
	bool 			m_stop;
};

template<typename T>
threadpool<T>::threadpool(int threadnum, size_t queuelen):
	m_threadnum(threadnum), m_max_reqnum(queuelen), m_stop(false)
{
	if((threadnum <= 0) && (queuelen >= 10000)){
		throw std::exception();
	}
	pth_array = new pthread_t[threadnum];
	if(!pth_array){
		throw std::exception();
	}
	m_worklocker.lock();
	for(int i=0; i<threadnum; ++i){
		if( pthread_create( pth_array + i, NULL, worker, this ) != 0 ){
			delete [] pth_array;
			throw std::exception();
		}
		if( pthread_detach( pth_array[i] ) ){
			delete [] pth_array;
			throw std::exception();
		}
	}
}

template<typename T>
threadpool<T>::~threadpool(){
	delete [] pth_array;
	m_stop = true;
}

template<typename T>
bool threadpool<T>::append(T request, bool isNew){

	m_queuelocker.lock();
	if(m_req_list.size() > m_max_reqnum){
		m_queuelocker.unlock();
		return false;
	}
	m_req_list.push_front(std::make_pair(request, isNew));
	m_queuelocker.unlock();
	m_queuestat.post();
	return true;
}

template<typename T>
void threadpool<T>::dowork()
{
	m_worklocker.lock();
	m_worklocker.unlock();
	pthread_t tid = pthread_self();
	while(!m_stop){
		m_queuestat.wait();
		m_queuelocker.lock();

		if(m_req_list.empty()){
			m_queuelocker.unlock();
			continue;
		}
		std::pair<T, bool> tmp = m_req_list.back();
		T request = tmp.first;
		bool isNew = tmp.second;
		m_req_list.pop_back();
		m_queuelocker.unlock();
		if(!request){
			continue;
		}
		request->process(tid, isNew);
	}
}

template<typename T>
void * threadpool<T>::worker(void * arg){

	threadpool * pool = static_cast<threadpool *>(arg);
	pool->dowork();
	return pool;
}

#endif /* THREADPOOL_HPP_ */
