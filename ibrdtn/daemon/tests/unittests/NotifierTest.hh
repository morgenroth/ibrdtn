/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        NotifierTest.hh
/// @brief       CPPUnit-Tests for class Notifier
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef NOTIFIERTEST_HH
#define NOTIFIERTEST_HH
class NotifierTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'Notifier' ===*/
		void testRaiseEvent();
		void testNotify();
		/*=== END   tests for class 'Notifier' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(NotifierTest);
			CPPUNIT_TEST(testRaiseEvent);
			CPPUNIT_TEST(testNotify);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* NOTIFIERTEST_HH */
