/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BundleEventTest.hh
/// @brief       CPPUnit-Tests for class BundleEvent
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef BUNDLEEVENTTEST_HH
#define BUNDLEEVENTTEST_HH
class BundleEventTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'BundleEvent' ===*/
		void testGetReason();
		void testGetAction();
		void testGetBundle();
		void testGetName();
		void testToString();
		void testRaise();
		/*=== END   tests for class 'BundleEvent' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(BundleEventTest);
			CPPUNIT_TEST(testGetReason);
			CPPUNIT_TEST(testGetAction);
			CPPUNIT_TEST(testGetBundle);
			CPPUNIT_TEST(testGetName);
			CPPUNIT_TEST(testToString);
			CPPUNIT_TEST(testRaise);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* BUNDLEEVENTTEST_HH */
