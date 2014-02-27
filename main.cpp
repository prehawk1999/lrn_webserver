/*
 * timer_test.cpp
 *
 *  Created on: Feb 11, 2014
 *      Author: prehawk
 */


#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include "timer.hpp"
#include <boost/assign.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include "threadpool.hpp"

using namespace std;
using boost::format;
using boost::shared_ptr;

#define SIZE 20
#define LINE_MAX 100;
static Timer * tm_hp;



void signal_handler(int sig){
	tm_hp->tick();
	alarm(1);
}

void addsignal(int sig){
	struct sigaction sa;
	memset(&sa, '\0', sizeof(sa));
	sa.sa_handler = signal_handler;
	sa.sa_flags |= SA_RESTART;
	sigfillset(&sa.sa_mask);
	assert(sigaction(sig, &sa, NULL) != -1);
}

class Req{
public:
	Req(int reqno):m_reqno(reqno){}
	inline void process(pthread_t threadno){
		cout << format("Thread %lu working request %d\n") % threadno % m_reqno;
		sleep(1);
	}
private:
	int m_reqno;
};

int main(int argc, char * argv[])
{


	shared_ptr< threadpool<Req> > pool
	= shared_ptr< threadpool<Req> >(new threadpool<Req>(2, 100));

	for(int i=0; i<5; ++i){
		Req * req = new Req(i);
		pool->append(req);
	}
	cout << format("request queue now have %d requests\n") % pool->queuesize();

	pool->run();

//	HttpParser hp = HttpParser();
//	char temp[LINE_MAX];
//	temp = "";
//	REQ_STATE ret = hp.parse(temp);
//	if(ret == REQ_OK){
//		cout << hp.response;
//	}
//	else{
//		cout << "request not complete" << endl;
//	}

	int i;
	cin >> i;
}
