/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        EventReceiverTest.hh
/// @brief       CPPUnit-Tests for class EventReceiver
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef EVENTRECEIVERTEST_HH
#define EVENTRECEIVERTEST_HH
class EventReceiverTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'EventReceiver' ===*/
		void testBindEvent();
		void testUnbindEvent();
		/*=== END   tests for class 'EventReceiver' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(EventReceiverTest);
			CPPUNIT_TEST(testBindEvent);
			CPPUNIT_TEST(testUnbindEvent);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* EVENTRECEIVERTEST_HH */
