/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        stopandwaitTest.hh
/// @brief       CPPUnit-Tests for class stopandwait
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef STOPANDWAITTEST_HH
#define STOPANDWAITTEST_HH
class stopandwaitTest : public CppUnit::TestFixture {
	private:
	public:
		void testRetransmission();

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(stopandwaitTest);
			CPPUNIT_TEST(testRetransmission);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* STOPANDWAITTEST_HH */
