/*
 * tcpstreamtest.cpp
 *
 *  Created on: 22.03.2010
 *      Author: morgenro
 */

#include "net/tcpclienttest.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/net/tcpclient.h"

CPPUNIT_TEST_SUITE_REGISTRATION (tcpclienttest);

void tcpclienttest :: setUp (void)
{
}

void tcpclienttest :: tearDown (void)
{
}

void tcpclienttest :: baseTest (void)
{
	ibrcommon::tcpclient client("www.google.de", 80);

	client << "GET /" << std::flush;

	client.close();
}

