/*
 * tcpstreamtest.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "net/tcpstreamtest.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/net/socketstream.h"

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

tcpstreamtest::StreamChecker::StreamChecker(int port, int chars)
 : _running(true), _srv(port), _chars(chars)
{
}

tcpstreamtest::StreamChecker::~StreamChecker()
{
	join();
}

void tcpstreamtest::runTest()
{
	StreamChecker _checker(4343, 10);

	char values[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	// start the streamchecker
	_checker.start();

	try {
		ibrcommon::vaddress addr("localhost");
		ibrcommon::socketstream client(new ibrcommon::tcpsocket(addr, 4343));

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
	} catch (const ibrcommon::socket_exception&) { };

	_checker.stop();
	_checker.join();
}

void tcpstreamtest::StreamChecker::__cancellation()
{
	_running = false;
	try {
		_srv.close();
	} catch (const ibrcommon::socket_exception&) {};
}

void tcpstreamtest::StreamChecker::setup() {
	_srv.up();
}

void tcpstreamtest::StreamChecker::run()
{
	char values[10] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	int byte = 0;

	ibrcommon::clientsocket *socket = NULL;

	while (_running) {
		try {
			ibrcommon::vaddress source;
			socket = _srv.accept(source);

			CPPUNIT_ASSERT(socket != NULL);

			// create a new stream
			ibrcommon::socketstream stream(socket);

			while (stream.good())
			{
				char value;

				// read one char
				stream.get(value);

				if ((value != values[byte % _chars]) && stream.good())
				{
					cout << "error in byte " << byte << ", " << value << " != " << values[byte % _chars] << endl;
					break;
				}

				byte++;
			}

			// connection should be closed
			CPPUNIT_ASSERT(stream.errmsg == ibrcommon::ERROR_CLOSED);

			stream.close();
		} catch (const ibrcommon::Exception&) { };
	}

	CPPUNIT_ASSERT(byte == 20000001);
}
