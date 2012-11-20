/*
 * DataStorageTest.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "storage/DataStorage.h"

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
