/*
 * timer.cpp
 *
 *  Created on: Feb 28, 2014
 *      Author: prehawk
 */


#include "heaptimer.h"

// 谁的expire时间大，谁优先级较小
bool operator<( const timer_unit & lhs, const timer_unit & rhs){

	if( lhs.expire > rhs.expire){
		return true;
	}
	return false;
}


void HeapTimer::add_delay_timer(time_t delay, std::string echo){
	tm_queue.push(timer_unit(time(NULL)+delay, echo));
}

void HeapTimer::add_expire_timer(time_t expire, std::string echo){
	tm_queue.push(timer_unit(expire, echo));
}

void HeapTimer::tick(){
	std::cout << "tick once" << std::endl;
	if(!empty()){
		timer_unit tm = tm_queue.top();
		time_t nowt = time(NULL);
		if(tm.expire < nowt){
			std::cout << tm.echo << std::endl;
			tm_queue.pop();
		}
	}

}
