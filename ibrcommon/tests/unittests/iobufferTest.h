/*
 * iobufferTest.h
 *
 *  Created on: 14.02.2011
 *      Author: morgenro
 */

#ifndef IOBUFFERTEST_H_
#define IOBUFFERTEST_H_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class iobufferTest : public CppUnit::TestFixture
{
public:
	void basicTest();

	void setUp();
	void tearDown();

	CPPUNIT_TEST_SUITE(iobufferTest);
	CPPUNIT_TEST(basicTest);
	CPPUNIT_TEST_SUITE_END();
};

#endif /* IOBUFFERTEST_H_ */
