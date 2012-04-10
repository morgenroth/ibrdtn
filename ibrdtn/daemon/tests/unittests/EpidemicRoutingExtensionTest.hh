/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        EpidemicRoutingExtensionTest.hh
/// @brief       CPPUnit-Tests for class EpidemicRoutingExtension
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef EPIDEMICROUTINGEXTENSIONTEST_HH
#define EPIDEMICROUTINGEXTENSIONTEST_HH
class EpidemicRoutingExtensionTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'EpidemicRoutingExtension' ===*/
		void testNotify();
		void testUpdate();
		/*=== BEGIN tests for class 'EpidemicExtensionBlock' ===*/
		void testGetSet();
		void testGetSetPurgeVector();
		void testGetSetSummaryVector();
		/*=== END   tests for class 'EpidemicExtensionBlock' ===*/

		void testRun();
		void test__cancellation();
		/*=== END   tests for class 'EpidemicRoutingExtension' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(EpidemicRoutingExtensionTest);
			CPPUNIT_TEST(testNotify);
			CPPUNIT_TEST(testUpdate);
			CPPUNIT_TEST(testGetSet);
			CPPUNIT_TEST(testGetSetPurgeVector);
			CPPUNIT_TEST(testGetSetSummaryVector);
			CPPUNIT_TEST(testRun);
			CPPUNIT_TEST(test__cancellation);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* EPIDEMICROUTINGEXTENSIONTEST_HH */
