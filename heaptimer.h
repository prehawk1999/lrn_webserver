/*
 * heap_timer2.hpp
 *
 *  Created on: Feb 13, 2014
 *      Author: prehawk
 */
//	usage:
//	HeapTimer tm = HeapTimer(80);
//	tm.add_delay_timer(5, "hello timer");
//	int i=100;
//	while(i--){
//		tm.tick();
//	}



#ifndef HEAP_TIMER2_HPP_
#define HEAP_TIMER2_HPP_
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <boost/shared_ptr.hpp>
#include <time.h>

class TimerComp;
//计时器单元类，
class timer_unit{

public:
	time_t expire;
	std::string echo;
	bool valid;
public:

	timer_unit( time_t delay, std::string hint):
		expire(delay), echo(hint), valid(true){}
};

class TimerComp
{
public:
	bool operator()(const timer_unit & lhs, const timer_unit & rhs) const{
		return lhs.expire > rhs.expire;
	}
};


// timer manager
class HeapTimer
{

public:
	HeapTimer(int capacity): m_cap(capacity){}
	void tick();
	void add_delay_timer(time_t delay, std::string echo);
	void add_expire_timer(time_t expire, std::string echo);
	size_t size(){return tm_queue.size();}
	bool empty(){return tm_queue.empty();}
private:
	int m_cap;
	std::priority_queue<timer_unit, std::vector<timer_unit>, TimerComp> tm_queue;
};



#endif /* HEAP_TIMER2_HPP_ */
