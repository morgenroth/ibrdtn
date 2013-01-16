/*
 * TestTrackingBlock.h
 *
 *  Created on: 16.01.2013
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TESTTRACKINGBLOCK_H_
#define TESTTRACKINGBLOCK_H_

class TestTrackingBlock : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestTrackingBlock);
	CPPUNIT_TEST (createTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void createTest(void);
};

#endif /* TESTTRACKINGBLOCK_H_ */
