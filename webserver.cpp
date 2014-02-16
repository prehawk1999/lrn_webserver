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
#include "heap_timer2.hpp"
#include <boost/assign.hpp>

using namespace std;
using namespace boost::assign;

#define SIZE 20
static hptimer * tm_hp;



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
int main(int argc, char * argv[])
{
	addsignal(SIGALRM);
	hptimer ht = hptimer(10);
	timer_unit t;

	ht.add_delay_timer( 12, "str4\n");
	ht.add_delay_timer( 2, "str2\n");
	ht.add_delay_timer( 1, "str1\n");
	ht.add_delay_timer( 1, "str1 too\n");
	ht.add_delay_timer( 6, "str3\n");
	for(int i=0; i<20; ++i){
		ht.tick();
		sleep(1);
	}


}
