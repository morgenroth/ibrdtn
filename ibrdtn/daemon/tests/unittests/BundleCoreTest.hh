/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BundleCoreTest.hh
/// @brief       CPPUnit-Tests for class BundleCore
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef BUNDLECORETEST_HH
#define BUNDLECORETEST_HH
class BundleCoreTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'BundleCore' ===*/
		void testGetInstance();
		void testGetClock();
		void testGetSetStorage();
		void testTransferTo();
		void testAddConvergenceLayer();
		void testGetNeighbors();
		void testRaiseEvent();
		/*=== END   tests for class 'BundleCore' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(BundleCoreTest);
			CPPUNIT_TEST(testGetInstance);
			CPPUNIT_TEST(testGetClock);
			CPPUNIT_TEST(testGetSetStorage);
			CPPUNIT_TEST(testTransferTo);
			CPPUNIT_TEST(testAddConvergenceLayer);
			CPPUNIT_TEST(testGetNeighbors);
			CPPUNIT_TEST(testRaiseEvent);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* BUNDLECORETEST_HH */
