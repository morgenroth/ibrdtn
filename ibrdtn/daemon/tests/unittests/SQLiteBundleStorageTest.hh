/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        SQLiteBundleStorageTest.hh
/// @brief       CPPUnit-Tests for class SQLiteBundleStorage
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef SQLITEBUNDLESTORAGETEST_HH
#define SQLITEBUNDLESTORAGETEST_HH
class SQLiteBundleStorageTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'SQLiteBundleStorage' ===*/
		void testStore();
		void testStoreBundleRoutingInfo();
		void testStoreNodeRoutingInfo();
		void testStoreRoutingInfo();
		void testGet();
		void testGetBundleRoutingInfo();
		void testGetNodeRoutingInfo();
		void testGetRoutingInfo();
		void testRemoveBundleRoutingInfo();
		void testRemoveNodeRoutingInfo();
		void testRemoveRoutingInfo();
		void testRemove();
		void testClear();
		void testClearAll();
		void testEmpty();
		void testCount();
		void testReleaseCustody();
		void testSetPriority();
		void testGetBundleBySize();
		void testGetBundleByTTL();
		void testGetBundlesBySource();
		void testGetBundlesByDestination();
		void testGetBlock();
		void testOccupiedSpace();
		void testRaiseEvent();
		void testRun();
		void test__cancellation();
		/*=== END   tests for class 'SQLiteBundleStorage' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(SQLiteBundleStorageTest);
			CPPUNIT_TEST(testStore);
			CPPUNIT_TEST(testStoreBundleRoutingInfo);
			CPPUNIT_TEST(testStoreNodeRoutingInfo);
			CPPUNIT_TEST(testStoreRoutingInfo);
			CPPUNIT_TEST(testGet);
			CPPUNIT_TEST(testGetBundleRoutingInfo);
			CPPUNIT_TEST(testGetNodeRoutingInfo);
			CPPUNIT_TEST(testGetRoutingInfo);
			CPPUNIT_TEST(testRemoveBundleRoutingInfo);
			CPPUNIT_TEST(testRemoveNodeRoutingInfo);
			CPPUNIT_TEST(testRemoveRoutingInfo);
			CPPUNIT_TEST(testRemove);
			CPPUNIT_TEST(testClear);
			CPPUNIT_TEST(testClearAll);
			CPPUNIT_TEST(testEmpty);
			CPPUNIT_TEST(testCount);
			CPPUNIT_TEST(testReleaseCustody);
			CPPUNIT_TEST(testSetPriority);
			CPPUNIT_TEST(testGetBundleBySize);
			CPPUNIT_TEST(testGetBundleByTTL);
			CPPUNIT_TEST(testGetBundlesBySource);
			CPPUNIT_TEST(testGetBundlesByDestination);
			CPPUNIT_TEST(testGetBlock);
			CPPUNIT_TEST(testOccupiedSpace);
			CPPUNIT_TEST(testRaiseEvent);
			CPPUNIT_TEST(testRun);
			CPPUNIT_TEST(test__cancellation);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* SQLITEBUNDLESTORAGETEST_HH */
