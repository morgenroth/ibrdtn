/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BaseRouterTest.hh
/// @brief       CPPUnit-Tests for class BaseRouter
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "storage/MemoryBundleStorage.h"

#ifndef BASEROUTERTEST_HH
#define BASEROUTERTEST_HH
class BaseRouterTest : public CppUnit::TestFixture {
	private:
		dtn::storage::MemoryBundleStorage _storage;

	public:
		/*=== BEGIN tests for class 'BaseRouter' ===*/
		/*=== BEGIN tests for class 'Extension' ===*/
		void testGetRouter();
		/*=== END   tests for class 'Extension' ===*/

		void testAddExtension();
		void testTransferTo();
		void testRaiseEvent();
		void testGetStorage();
		void testIsKnown();
		void testSetKnown();
		void testGetSummaryVector();
		/*=== END   tests for class 'BaseRouter' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(BaseRouterTest);
			CPPUNIT_TEST(testGetRouter);
			CPPUNIT_TEST(testAddExtension);
			CPPUNIT_TEST(testTransferTo);
			CPPUNIT_TEST(testRaiseEvent);
			CPPUNIT_TEST(testGetStorage);
			CPPUNIT_TEST(testIsKnown);
			CPPUNIT_TEST(testSetKnown);
			CPPUNIT_TEST(testGetSummaryVector);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* BASEROUTERTEST_HH */
