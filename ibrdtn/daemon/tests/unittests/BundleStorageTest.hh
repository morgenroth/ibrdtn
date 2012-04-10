/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BundleStorageTest.hh
/// @brief       CPPUnit-Tests for class BundleStorage
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef BUNDLESTORAGETEST_HH
#define BUNDLESTORAGETEST_HH
class BundleStorageTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'BundleStorage' ===*/
		void testRemove();
		void testAcceptCustody();
		void testRejectCustody();
		/*=== END   tests for class 'BundleStorage' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(BundleStorageTest);
			CPPUNIT_TEST(testRemove);
			CPPUNIT_TEST(testAcceptCustody);
			CPPUNIT_TEST(testRejectCustody);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* BUNDLESTORAGETEST_HH */
