/*
 * iobufferTest.cpp
 *
 *  Created on: 14.02.2011
 *      Author: morgenro
 */

#include "iobufferTest.h"
#include <ibrcommon/data/iobuffer.h>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION(iobufferTest);

void iobufferTest::setUp()
{
}

void iobufferTest::tearDown()
{
}

void iobufferTest::basicTest()
{
	ibrcommon::iobuffer buf;
	std::istream is(&buf);
	std::ostream os(&buf);

	os << "Hallo Welt" << std::flush;
	buf.finalize();

	std::stringstream ss; ss << is.rdbuf();

	CPPUNIT_ASSERT_EQUAL(ss.str(), std::string("Hallo Welt"));
}
