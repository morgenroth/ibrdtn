/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        TimerTest.hh
/// @brief       CPPUnit-Tests for class Timer
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TIMERTEST_HH
#define TIMERTEST_HH
class TimerTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'Timer' ===*/
		void testGet_current_time();
		void testSet();
		void testReset();
		void testRun();
		void test__cancellation();
		/*=== END   tests for class 'Timer' ===*/

		/*=== BEGIN tests for class 'SimpleTimer' ===*/
		void testSimpleTimerSet();
		void testSimpleTimerRemove();
		void testSimpleTimerRun();
		void testSimpleTimer__cancellation();
		/*=== END   tests for class 'SimpleTimer' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(TimerTest);
			CPPUNIT_TEST(testGet_current_time);
			CPPUNIT_TEST(testSet);
			CPPUNIT_TEST(testReset);
			CPPUNIT_TEST(testRun);
			CPPUNIT_TEST(test__cancellation);
			CPPUNIT_TEST(testSimpleTimerSet);
			CPPUNIT_TEST(testSimpleTimerRemove);
			CPPUNIT_TEST(testSimpleTimerRun);
			CPPUNIT_TEST(testSimpleTimer__cancellation);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* TIMERTEST_HH */
