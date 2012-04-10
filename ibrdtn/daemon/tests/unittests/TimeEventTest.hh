/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        TimeEventTest.hh
/// @brief       CPPUnit-Tests for class TimeEvent
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TIMEEVENTTEST_HH
#define TIMEEVENTTEST_HH
class TimeEventTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'TimeEvent' ===*/
		void testGetAction();
		void testGetTimestamp();
		void testGetName();
		void testRaise();
		void testToString();
		/*=== END   tests for class 'TimeEvent' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(TimeEventTest);
			CPPUNIT_TEST(testGetAction);
			CPPUNIT_TEST(testGetTimestamp);
			CPPUNIT_TEST(testGetName);
			CPPUNIT_TEST(testRaise);
			CPPUNIT_TEST(testToString);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* TIMEEVENTTEST_HH */
