/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        udpsocketTest.cpp
/// @brief       CPPUnit-Tests for class udpsocket
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

/*
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

#include "stopandwaitTest.hh"
#include <ibrcommon/net/stopandwait.h>


CPPUNIT_TEST_SUITE_REGISTRATION(stopandwaitTest);

/*========================== tests below ==========================*/

void stopandwaitTest::testRetransmission()
{
	class CommPeer : private ibrcommon::stopandwait
	{
	public:
		CommPeer() : ibrcommon::stopandwait(100, 5), _sent_bytes(0)
		{ }

		virtual ~CommPeer() {};

		int send()
		{
			std::string data = "Hello World!";
			return __send(data.c_str(), data.length());
		}

		int __send_impl(const char *buffer, const size_t length)
		{
			_sent_bytes += length;
			return 0;
		}

		int __recv_impl(char *buffer, size_t &length)
		{
			return 0;
		}

		int _sent_bytes;
	};

	CommPeer cp;

	cp.send();
	CPPUNIT_ASSERT_EQUAL(14 * 6, cp._sent_bytes);
}

void stopandwaitTest::setUp()
{
}

void stopandwaitTest::tearDown()
{
}

