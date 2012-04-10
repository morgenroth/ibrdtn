/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        SummaryVectorTest.hh
/// @brief       CPPUnit-Tests for class SummaryVector
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef SUMMARYVECTORTEST_HH
#define SUMMARYVECTORTEST_HH
class SummaryVectorTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'SummaryVector' ===*/
		void testGetLength();
		void testGetBloomFilter();
		void testGetNotIn();
		void testOperatorShiftLeft();
		void testOperatorShiftRight();
		/*=== END   tests for class 'SummaryVector' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(SummaryVectorTest);
			CPPUNIT_TEST(testGetLength);
			CPPUNIT_TEST(testGetBloomFilter);
			CPPUNIT_TEST(testGetNotIn);
			CPPUNIT_TEST(testOperatorShiftLeft);
			CPPUNIT_TEST(testOperatorShiftRight);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* SUMMARYVECTORTEST_HH */
