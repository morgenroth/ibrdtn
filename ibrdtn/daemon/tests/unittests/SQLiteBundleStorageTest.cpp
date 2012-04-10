/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        SQLiteBundleStorageTest.cpp
/// @brief       CPPUnit-Tests for class SQLiteBundleStorage
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 

#include "SQLiteBundleStorageTest.hh"



CPPUNIT_TEST_SUITE_REGISTRATION(SQLiteBundleStorageTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'SQLiteBundleStorage' ===*/
void SQLiteBundleStorageTest::testStore()
{
	/* test signature (const dtn::data::Bundle &bundle) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testStoreBundleRoutingInfo()
{
	/* test signature (const data::BundleID &BundleID, const int &key, const string &routingInfo) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testStoreNodeRoutingInfo()
{
	/* test signature (const data::EID &nodel, const int &key, const std::string &routingInfo) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testStoreRoutingInfo()
{
	/* test signature (const int &key, const std::string &routingInfo) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testGet()
{
	/* test signature (const dtn::data::BundleID &id) */
	/* test signature (const dtn::data::EID &eid) */
	/* test signature (const ibrcommon::BloomFilter &filter) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testGetBundleRoutingInfo()
{
	/* test signature (const data::BundleID &bundleID, const int &key) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testGetNodeRoutingInfo()
{
	/* test signature (const data::EID &eid, const int &key) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testGetRoutingInfo()
{
	/* test signature (const int &key) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testRemoveBundleRoutingInfo()
{
	/* test signature (const data::BundleID &bundleID, const int &key) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testRemoveNodeRoutingInfo()
{
	/* test signature (const data::EID &eid, const int &key) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testRemoveRoutingInfo()
{
	/* test signature (const int &key) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testRemove()
{
	/* test signature (const dtn::data::BundleID &id) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testClear()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testClearAll()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testEmpty()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testCount()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testReleaseCustody()
{
	/* test signature (dtn::data::BundleID &bundle) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testSetPriority()
{
	/* test signature (const int priority,const dtn::data::BundleID &id) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testGetBundleBySize()
{
	/* test signature (const int &limit) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testGetBundleByTTL()
{
	/* test signature (const int &limit) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testGetBundlesBySource()
{
	/* test signature (const dtn::data::EID &sourceID) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testGetBundlesByDestination()
{
	/* test signature (const dtn::data::EID &sourceID) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testGetBlock()
{
	/* test signature (const data::BundleID &bundleID,const char &blocktype) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testOccupiedSpace()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testRaiseEvent()
{
	/* test signature (const Event *evt) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::testRun()
{
	/* test signature (void) */
	CPPUNIT_FAIL("not implemented");
}

void SQLiteBundleStorageTest::test__cancellation()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'SQLiteBundleStorage' ===*/

void SQLiteBundleStorageTest::setUp()
{
}

void SQLiteBundleStorageTest::tearDown()
{
}

