/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        EventSwitchTest.hh
/// @brief       CPPUnit-Tests for class EventSwitch
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef EVENTSWITCHTEST_HH
#define EVENTSWITCHTEST_HH
class EventSwitchTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'EventSwitch' ===*/
		void testRegisterEventReceiver();
		void testUnregisterEventReceiver();
		void testRaiseEvent();
		void testGetInstance();
		void testLoop();
		/*=== END   tests for class 'EventSwitch' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(EventSwitchTest);
			CPPUNIT_TEST(testRegisterEventReceiver);
			CPPUNIT_TEST(testUnregisterEventReceiver);
			CPPUNIT_TEST(testRaiseEvent);
			CPPUNIT_TEST(testGetInstance);
			CPPUNIT_TEST(testLoop);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* EVENTSWITCHTEST_HH */
