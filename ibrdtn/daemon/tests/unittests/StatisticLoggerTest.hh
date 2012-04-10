/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        StatisticLoggerTest.hh
/// @brief       CPPUnit-Tests for class StatisticLogger
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef STATISTICLOGGERTEST_HH
#define STATISTICLOGGERTEST_HH
class StatisticLoggerTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'StatisticLogger' ===*/
		void testComponentUp();
		void testComponentDown();
		void testTimeout();
		void testRaiseEvent();
		/*=== END   tests for class 'StatisticLogger' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(StatisticLoggerTest);
			CPPUNIT_TEST(testComponentUp);
			CPPUNIT_TEST(testComponentDown);
			CPPUNIT_TEST(testTimeout);
			CPPUNIT_TEST(testRaiseEvent);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* STATISTICLOGGERTEST_HH */
