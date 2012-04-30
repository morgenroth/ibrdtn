/*
 * TestUtils.h
 *
 *  Created on: 30.04.2012
 *      Author: roettger
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TESTUTILS_H_
#define TESTUTILS_H_

class TestUtils : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestUtils);
	CPPUNIT_TEST (tokenizeTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void tokenizeTest(void);
};


#endif /* TESTUTILS_H_ */

