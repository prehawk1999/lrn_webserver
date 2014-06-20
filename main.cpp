/*
 * timer_test.cpp
 *
 *  Created on: Feb 11, 2014
 *      Author: prehawk
 */
#include "WebServer.hpp"
#include "HttpRequest.h"
#include "Request.h"
#include "IOwrapper.hpp"


int main(int argc, char * argv[])
{
	WebServer<HttpRequest> wb = WebServer<HttpRequest>("127.0.0.1", 8080, 16, 10000);
	wb.run();
}
