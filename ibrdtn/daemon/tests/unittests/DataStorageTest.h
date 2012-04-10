/*
 * DataStorageTest.h
 *
 *  Created on: 22.11.2010
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "src/storage/DataStorage.h"

#ifndef DATASTORAGETEST_H_
#define DATASTORAGETEST_H_

class DataStorageTest : public CppUnit::TestFixture
{
public:
	void testHashTest();
	void testStoreTest();
	void testRemoveTest();
	void testStressTest();

	void setUp();
	void tearDown();

	CPPUNIT_TEST_SUITE(DataStorageTest);
	CPPUNIT_TEST(testHashTest);
	CPPUNIT_TEST(testStoreTest);
	CPPUNIT_TEST(testRemoveTest);
	CPPUNIT_TEST(testStressTest);
	CPPUNIT_TEST_SUITE_END();

private:
	class DataCallbackDummy : public dtn::storage::DataStorage::Callback
	{
	public:
		DataCallbackDummy() {};
		virtual ~DataCallbackDummy() {};

		void eventDataStorageStored(const dtn::storage::DataStorage::Hash&) {};
		void eventDataStorageStoreFailed(const dtn::storage::DataStorage::Hash&, const ibrcommon::Exception&) {};
		void eventDataStorageRemoved(const dtn::storage::DataStorage::Hash&) {};
		void eventDataStorageRemoveFailed(const dtn::storage::DataStorage::Hash&, const ibrcommon::Exception&) {};
		void iterateDataStorage(const dtn::storage::DataStorage::Hash&, dtn::storage::DataStorage::istream&) {};
	};
};

#endif /* DATASTORAGETEST_H_ */
