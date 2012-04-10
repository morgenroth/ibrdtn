/*
 * tcpstreamtest.h
 *
 *  Created on: 22.03.2010
 *      Author: morgenro
 */

#ifndef TCPCLIENTTEST_H_
#define TCPCLIENTTEST_H_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace std;

class tcpclienttest : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (tcpclienttest);
	CPPUNIT_TEST (baseTest);
	CPPUNIT_TEST_SUITE_END ();

	public:
		void setUp (void);
		void tearDown (void);

	protected:
		void baseTest (void);
};

#endif /* TCPCLIENTTEST_H_ */
