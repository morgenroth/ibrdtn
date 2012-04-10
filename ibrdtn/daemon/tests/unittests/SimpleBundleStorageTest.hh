/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        SimpleBundleStorageTest.hh
/// @brief       CPPUnit-Tests for class SimpleBundleStorage
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "src/storage/SimpleBundleStorage.h"
#include "src/storage/MemoryBundleStorage.h"

#ifndef SIMPLEBUNDLESTORAGETEST_HH
#define SIMPLEBUNDLESTORAGETEST_HH
class SimpleBundleStorageTest : public CppUnit::TestFixture {
	private:
		void completeTest(dtn::storage::BundleStorage &storage);
		void concurrentStoreGet(dtn::storage::BundleStorage &storage);

	public:
		/*=== BEGIN tests for class 'SimpleBundleStorage' ===*/
		void testGetList();
		void testRemove();
		void testClear();
		void testEmpty();
		void testCount();
		void testSize();
		void testReleaseCustody();
		void testRaiseEvent();
		/*=== END   tests for class 'SimpleBundleStorage' ===*/

		void testMemoryTest();
		void testDiskTest();
		void testConcurrentMemory();
		void testConcurrentDisk();
		void testDiskRestore();


		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(SimpleBundleStorageTest);
			CPPUNIT_TEST(testGetList);
			CPPUNIT_TEST(testRemove);
			CPPUNIT_TEST(testClear);
			CPPUNIT_TEST(testEmpty);
			CPPUNIT_TEST(testCount);
			CPPUNIT_TEST(testSize);
			CPPUNIT_TEST(testReleaseCustody);
			CPPUNIT_TEST(testRaiseEvent);
			CPPUNIT_TEST(testMemoryTest);
			CPPUNIT_TEST(testDiskTest);
			CPPUNIT_TEST(testConcurrentMemory);
			CPPUNIT_TEST(testConcurrentDisk);
			CPPUNIT_TEST(testDiskRestore);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* SIMPLEBUNDLESTORAGETEST_HH */
