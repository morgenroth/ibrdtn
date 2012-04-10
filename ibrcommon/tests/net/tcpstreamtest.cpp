/*
 * tcpstreamtest.cpp
 *
 *  Created on: 22.03.2010
 *      Author: morgenro
 */

#include "net/tcpstreamtest.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/net/tcpclient.h"

CPPUNIT_TEST_SUITE_REGISTRATION (tcpstreamtest);

void tcpstreamtest :: setUp (void)
{
}

void tcpstreamtest :: tearDown (void)
{
}

void tcpstreamtest :: baseTest (void)
{
	for (int i = 0; i < 10; i++)
	{
		runTest();
	}
}

tcpstreamtest::StreamChecker::StreamChecker(int chars)
 : _srv(), _chars(chars)
{
	_srv.bind(4343);
}

tcpstreamtest::StreamChecker::~StreamChecker()
{
	join();
}

void tcpstreamtest::runTest()
{
	_checker = new StreamChecker(10);

	char values[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	// start the streamchecker
	_checker->start();

	try {
		ibrcommon::tcpclient client("127.0.0.1", 4343);

		// send some data
		for (size_t j = 0; j < 20; j++)
		{
			for (size_t i = 0; i < 100000; i++)
			{
				for (int k = 0; k < 10; k++)
				{
					client.put(values[k]);
				}
			}
		}

		client.flush();
		client.close();
	} catch (const ibrcommon::tcpclient::SocketException&) { };

	_checker->stop();
	_checker->join();

	delete _checker;
}

void tcpstreamtest::StreamChecker::__cancellation()
{
	_srv.shutdown();
	_srv.close();
}

void tcpstreamtest::StreamChecker::run()
{
	char values[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	int byte = 0;

	ibrcommon::tcpstream *stream = NULL;

	try {
		stream = _srv.accept();

		CPPUNIT_ASSERT(stream != NULL);

		while (stream->good())
		{
			char value;

			// read one char
			stream->get(value);

			if ((value != values[byte % _chars]) && stream->good())
			{
				cout << "error in byte " << byte << ", " << value << " != " << values[byte % _chars] << endl;
				break;
			}

			byte++;
		}

		// connection should be closed
		CPPUNIT_ASSERT(stream->errmsg == ibrcommon::tcpstream::ERROR_CLOSED);

		stream->close();
	} catch (const ibrcommon::Exception&) { };

	CPPUNIT_ASSERT(byte == 20000001);

	delete stream;
}
