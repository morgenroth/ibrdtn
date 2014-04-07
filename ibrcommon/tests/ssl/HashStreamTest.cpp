/*
 * HashStreamTest.cpp
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

#include "ssl/HashStreamTest.h"

#include <ibrcommon/ssl/HMacStream.h>
#include <ibrcommon/ssl/MD5Stream.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION (HashStreamTest);

void HashStreamTest::setUp()
{
}

void HashStreamTest::tearDown()
{
}

std::string HashStreamTest::getHex(std::istream &stream)
{
	std::stringstream buf;
	unsigned int c = stream.get();

	while (!stream.eof())
	{
		buf << std::hex << c << ":";
		c = stream.get();
	}

	buf << std::flush;

	return buf.str();
}

void HashStreamTest::hmacstream_test01()
{
	{
		std::string key = "1234557890123456";
		ibrcommon::HMacStream hstream((unsigned char*)key.c_str(), key.length());
		hstream << "Hello World" << std::flush;
		if ("b3:7a:c5:9f:6d:2:60:bd:da:51:cc:d3:95:2:11:7:c0:f1:7b:f9:" != getHex(hstream))
		{
			throw ibrcommon::Exception("unexpected hash value");
		}
	}

	{
		std::string key = "1234557890123456";
		ibrcommon::HMacStream hstream((unsigned char*)key.c_str(), key.length());
		hstream << "Hello again my World!" << std::flush;
		if ("a0:1d:99:f2:99:a4:b9:dc:b7:44:df:7b:b5:75:19:c6:20:8:bc:da:" != getHex(hstream))
		{
			throw ibrcommon::Exception("unexpected hash value");
		}
	}

}

void HashStreamTest::md5stream_test02()
{
	std::string hash_data("0123456789");

	ibrcommon::MD5Stream md5;
	md5 << hash_data << std::flush;

	if ("78:1e:5e:24:5d:69:b5:66:97:9b:86:e2:8d:23:f2:c7:" != getHex(md5))
	{
		throw ibrcommon::Exception("unexpected hash value");
	}
}
