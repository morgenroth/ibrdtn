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

#include "net/tcpclienttest.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/net/vaddress.h"
#include "ibrcommon/net/socketstream.h"
#include "ibrcommon/net/socket.h"

CPPUNIT_TEST_SUITE_REGISTRATION (tcpclienttest);

void tcpclienttest :: setUp (void)
{
}

void tcpclienttest :: tearDown (void)
{
}

void tcpclienttest :: baseTest (void)
{
	ibrcommon::vaddress addr("www.google.de", 80);
	ibrcommon::socketstream client(new ibrcommon::tcpsocket(addr));

	client << "GET /" << std::flush;

	client.close();
}

