/*
 * HashStreamTest.cpp
 *
 *  Created on: 12.07.2010
 *      Author: morgenro
 */

#include "ssl/HashStreamTest.h"

#include <ibrcommon/ssl/HMacStream.h>
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
