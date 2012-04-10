/*
 * TestSecurityBlock.h
 *
 *  Created on: 10.01.2011
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TESTSECURITYBLOCK_H_
#define TESTSECURITYBLOCK_H_

class TestSecurityBlock : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestSecurityBlock);
	CPPUNIT_TEST (localBABTest);
	CPPUNIT_TEST (serializeBABTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void localBABTest(void);
	void serializeBABTest(void);
};

#endif /* TESTSECURITYBLOCK_H_ */
