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


#include <iostream>
#include <sstream>
#include <ios>


int main(int argc, char * argv[])
{

	WebServer<HttpParser> wb = WebServer<HttpParser>("127.0.0.1", 6543, 2, 10000);
	wb.run();

	int i;
	std::cin >> i;
}
