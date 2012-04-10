/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        TimeMeasurementTest.hh
/// @brief       CPPUnit-Tests for class TimeMeasurement
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TIMEMEASUREMENTTEST_HH
#define TIMEMEASUREMENTTEST_HH
class TimeMeasurementTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'TimeMeasurement' ===*/
		void testStart();
		void testStop();
		void testGetMilliseconds();
		void testGetSeconds();
		void testOperatorShiftLeft();
		/*=== END   tests for class 'TimeMeasurement' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(TimeMeasurementTest);
			CPPUNIT_TEST(testStart);
			CPPUNIT_TEST(testStop);
			CPPUNIT_TEST(testGetMilliseconds);
			CPPUNIT_TEST(testGetSeconds);
			CPPUNIT_TEST(testOperatorShiftLeft);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* TIMEMEASUREMENTTEST_HH */
