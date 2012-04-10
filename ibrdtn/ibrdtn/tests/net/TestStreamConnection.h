/*
 * TestStreamConnection.h
 *
 *  Created on: 04.11.2010
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TESTSTREAMCONNECTION_H_
#define TESTSTREAMCONNECTION_H_

class TestStreamConnection : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestStreamConnection);
	CPPUNIT_TEST (connectionUpDown);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void connectionUpDown(void);
};


#endif /* TESTSTREAMCONNECTION_H_ */
