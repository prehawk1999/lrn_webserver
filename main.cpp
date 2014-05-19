/*
 * timer_test.cpp
 *
 *  Created on: Feb 11, 2014
 *      Author: prehawk
 */
#include "WebServer.hpp"
#include "HttpParser.h"
#include "Request.h"
#include "IOwrapper.hpp"

class A{
public:
	int a;
	int b;
	A create(){
		A tmp = A();
		tmp.a = 1;
		tmp.b = 2;
		return tmp;
	}
};

class B: private A{
public:
	int c;
};


int main(int argc, char * argv[])
{
	WebServer<HttpParser> wb = WebServer<HttpParser>("127.0.0.1", 6543, 4, 10000);
	wb.run();
}
