/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        EventTest.hh
/// @brief       CPPUnit-Tests for class Event
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef EVENTTEST_HH
#define EVENTTEST_HH
class EventTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'Event' ===*/
		void testRaiseEvent();
		/*=== END   tests for class 'Event' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(EventTest);
			CPPUNIT_TEST(testRaiseEvent);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* EVENTTEST_HH */
