/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        EventDebuggerTest.hh
/// @brief       CPPUnit-Tests for class EventDebugger
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef EVENTDEBUGGERTEST_HH
#define EVENTDEBUGGERTEST_HH
class EventDebuggerTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'EventDebugger' ===*/
		void testRaiseEvent();
		/*=== END   tests for class 'EventDebugger' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(EventDebuggerTest);
			CPPUNIT_TEST(testRaiseEvent);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* EVENTDEBUGGERTEST_HH */
