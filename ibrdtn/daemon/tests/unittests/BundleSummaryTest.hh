/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BundleSummaryTest.hh
/// @brief       CPPUnit-Tests for class BundleSummary
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "routing/BundleSummary.h"
#include <iostream>

#ifndef BUNDLESUMMARYTEST_HH
#define BUNDLESUMMARYTEST_HH
class BundleSummaryTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'BundleSummary' ===*/
		void testAdd();
		void testRemove();
		void testClear();
		void testContains();
		void testGetSummaryVector();
		/*=== END   tests for class 'BundleSummary' ===*/

		void expireTest(void);

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(BundleSummaryTest);
			CPPUNIT_TEST(testAdd);
			CPPUNIT_TEST(testRemove);
			CPPUNIT_TEST(testClear);
			CPPUNIT_TEST(testContains);
			CPPUNIT_TEST(testGetSummaryVector);
			CPPUNIT_TEST(expireTest);
		CPPUNIT_TEST_SUITE_END();

	private:
		void genbundles(dtn::routing::BundleSummary &l, int number, int offset, int max);
		std::string getHex(std::istream &stream);
};
#endif /* BUNDLESUMMARYTEST_HH */
