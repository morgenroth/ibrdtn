/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        WallClockTest.hh
/// @brief       CPPUnit-Tests for class WallClock
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef WALLCLOCKTEST_HH
#define WALLCLOCKTEST_HH
class WallClockTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'WallClock' ===*/
		void testSync();
		/*=== END   tests for class 'WallClock' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(WallClockTest);
			CPPUNIT_TEST(testSync);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* WALLCLOCKTEST_HH */
