/*
 * heap_timer2.hpp
 *
 *  Created on: Feb 13, 2014
 *      Author: prehawk
 */
//	usage:
//	Timer tm = Timer(80);
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

//计时器单元类，
class timer_unit{
public:
	time_t expire;
	std::string echo;

	timer_unit( time_t delay, std::string hint):
		expire(delay), echo(hint), m_valid(true){}

	timer_unit(): m_valid(false), expire(0), echo(""){}

	bool valid(){
		return m_valid;
	}
private:
	bool m_valid;
};

class Timer{

public:
	Timer(int capacity): m_cap(capacity){}
	void tick();
	void add_delay_timer(time_t delay, std::string echo);
	void add_expire_timer(time_t expire, std::string echo);
	size_t size(){return tm_queue.size();}
	bool empty(){return tm_queue.empty();}

private:
	std::vector<timer_unit> tm_array;
	std::priority_queue<timer_unit> tm_queue;
	int m_cap;
};

//谁的expire时间大，谁优先级较小
bool operator<(const timer_unit & lhs, const timer_unit & rhs){

	if(lhs.expire > rhs.expire){
		return true;
	}
	return false;
}

void Timer::add_delay_timer(time_t delay, std::string echo){
	tm_queue.push(timer_unit(time(NULL)+delay, echo));
}

void Timer::add_expire_timer(time_t expire, std::string echo){
	tm_queue.push(timer_unit(expire, echo));
}

void Timer::tick(){
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
#endif /* HEAP_TIMER2_HPP_ */
