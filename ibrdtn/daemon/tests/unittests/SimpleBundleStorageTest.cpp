/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        SimpleBundleStorageTest.cpp
/// @brief       CPPUnit-Tests for class SimpleBundleStorage
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

/*
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

#include "SimpleBundleStorageTest.hh"
#include "../tools/EventSwitchLoop.h"
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>
#include <ibrcommon/thread/Thread.h>
#include "core/TimeEvent.h"
#include <ibrdtn/utils/Clock.h>
#include "core/BundleCore.h"
#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrdtn/data/PayloadBlock.h>
#include "Component.h"


CPPUNIT_TEST_SUITE_REGISTRATION(SimpleBundleStorageTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'SimpleBundleStorage' ===*/
void SimpleBundleStorageTest::testGetList()
{
	/* test signature () */
	dtn::storage::MemoryBundleStorage storage;
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://node-one/test");

	CPPUNIT_ASSERT_EQUAL((unsigned int)0, storage.count());

	storage.store(b);

	CPPUNIT_ASSERT_EQUAL((unsigned int)1, storage.count());
}

void SimpleBundleStorageTest::testRemove()
{
	/* test signature (const dtn::data::BundleID &id) */
	/* test signature (const ibrcommon::BloomFilter &filter) */
	dtn::storage::MemoryBundleStorage storage;
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://node-one/test");

	CPPUNIT_ASSERT_EQUAL((unsigned int)0, storage.count());

	storage.store(b);

	CPPUNIT_ASSERT_EQUAL((unsigned int)1, storage.count());

	storage.remove(b);

	CPPUNIT_ASSERT_EQUAL((unsigned int)0, storage.count());
}

void SimpleBundleStorageTest::testClear()
{
	/* test signature () */
	dtn::storage::MemoryBundleStorage storage;
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://node-one/test");

	CPPUNIT_ASSERT_EQUAL((unsigned int)0, storage.count());

	storage.store(b);

	CPPUNIT_ASSERT_EQUAL((unsigned int)1, storage.count());

	storage.clear();

	CPPUNIT_ASSERT_EQUAL((unsigned int)0, storage.count());
}

void SimpleBundleStorageTest::testEmpty()
{
	/* test signature () */
	dtn::storage::MemoryBundleStorage storage;
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://node-one/test");

	CPPUNIT_ASSERT_EQUAL(true, storage.empty());

	storage.store(b);

	CPPUNIT_ASSERT_EQUAL(false, storage.empty());

	storage.clear();

	CPPUNIT_ASSERT_EQUAL(true, storage.empty());
}

void SimpleBundleStorageTest::testCount()
{
	/* test signature () */
	dtn::storage::MemoryBundleStorage storage;
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://node-one/test");

	CPPUNIT_ASSERT_EQUAL((unsigned int)0, storage.count());

	storage.store(b);

	CPPUNIT_ASSERT_EQUAL((unsigned int)1, storage.count());
}

void SimpleBundleStorageTest::testSize()
{
	/* test signature () const */
	dtn::storage::MemoryBundleStorage storage;
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://node-one/test");

	CPPUNIT_ASSERT_EQUAL((size_t)0, storage.size());

	storage.store(b);

	std::stringstream ss;
	dtn::data::DefaultSerializer(ss) << b;

	CPPUNIT_ASSERT_EQUAL((size_t)ss.str().length(), storage.size());
}

void SimpleBundleStorageTest::testReleaseCustody()
{
	/* test signature (dtn::data::BundleID &bundle) */
	dtn::storage::MemoryBundleStorage storage;
	dtn::data::BundleID id;
	dtn::data::EID eid;
	storage.releaseCustody(eid, id);
}

void SimpleBundleStorageTest::testRaiseEvent()
{
	ibrtest::EventSwitchLoop esl; esl.start();

		/* test signature (const Event *evt) */
	dtn::storage::MemoryBundleStorage storage;
	dtn::data::EID eid("dtn://node-one/test");
	dtn::data::Bundle b;
	b._lifetime = 60;
	b._source = dtn::data::EID("dtn://node-two/foo");

	storage.initialize();
	storage.startup();
	storage.store(b);
	dtn::core::TimeEvent::raise(dtn::utils::Clock::getTime() + 3600, dtn::utils::Clock::getTime() + 3600 - dtn::utils::Clock::TIMEVAL_CONVERSION, dtn::core::TIME_SECOND_TICK);

	dtn::core::GlobalEvent::raise(dtn::core::GlobalEvent::GLOBAL_SHUTDOWN);
	esl.join();

	storage.terminate();
}

/*=== END   tests for class 'SimpleBundleStorage' ===*/

void SimpleBundleStorageTest::setUp()
{
}

void SimpleBundleStorageTest::tearDown()
{
}

void SimpleBundleStorageTest::testMemoryTest()
{
	dtn::storage::MemoryBundleStorage storage;
	completeTest(storage);
}

void SimpleBundleStorageTest::testDiskTest()
{
	ibrcommon::File workdir("/tmp/bundle-disk-test");
	if (workdir.exists()) workdir.remove(true);
	ibrcommon::File::createDirectory(workdir);

	dtn::storage::SimpleBundleStorage storage(workdir);
	completeTest(storage);
}

void SimpleBundleStorageTest::testConcurrentMemory()
{
	dtn::storage::MemoryBundleStorage storage;
	concurrentStoreGet(storage);
}

void SimpleBundleStorageTest::testConcurrentDisk()
{
	ibrcommon::File workdir("/tmp/bundle-disk-test");
	if (workdir.exists()) workdir.remove(true);
	ibrcommon::File::createDirectory(workdir);

	dtn::storage::SimpleBundleStorage storage(workdir);
	concurrentStoreGet(storage);
}

void SimpleBundleStorageTest::completeTest(dtn::storage::BundleStorage &storage)
{
	ibrtest::EventSwitchLoop esl;
	dtn::core::BundleCore &core = dtn::core::BundleCore::getInstance();
	esl.start();
	core.initialize();

	try {
		dtn::daemon::Component &component = dynamic_cast<dtn::daemon::Component&>(storage);
		component.initialize();
		component.startup();
	} catch (const std::bad_cast&) { };

	dtn::data::EID eid("dtn://node-one/test");
	dtn::data::Bundle b;
	b._lifetime = 1;
	b._source = dtn::data::EID("dtn://node-two/foo");

	storage.store(b);

	dtn::core::GlobalEvent::raise(dtn::core::GlobalEvent::GLOBAL_SHUTDOWN);
	esl.join();

	CPPUNIT_ASSERT_EQUAL((unsigned int)1, storage.count());

	try {
		dtn::daemon::Component &component = dynamic_cast<dtn::daemon::Component&>(storage);
		component.terminate();
	} catch (const std::bad_cast&) { };

	core.terminate();
}

void SimpleBundleStorageTest::concurrentStoreGet(dtn::storage::BundleStorage &storage)
{
	class StoreProcess : public ibrcommon::JoinableThread
	{
	public:
		StoreProcess(dtn::storage::BundleStorage &storage, std::list<dtn::data::Bundle> &list)
		: _storage(storage), _list(list)
		{ };

		virtual ~StoreProcess()
		{
			join();
		};

		void __cancellation() throw ()
		{
		}

	protected:
		void run() throw ()
		{
			for (std::list<dtn::data::Bundle>::const_iterator iter = _list.begin(); iter != _list.end(); iter++)
			{
				const dtn::data::Bundle &b = (*iter);
				_storage.store(b);
			}
		}

	private:
		dtn::storage::BundleStorage &_storage;
		std::list<dtn::data::Bundle> &_list;
	};

	ibrtest::EventSwitchLoop esl;
	dtn::core::BundleCore &core = dtn::core::BundleCore::getInstance();

	// startup all services
	esl.start();
	core.initialize();

	try {
		dtn::daemon::Component &component = dynamic_cast<dtn::daemon::Component&>(storage);
		component.initialize();
		component.startup();
	} catch (const std::bad_cast&) { };

	// create some bundles
	std::list<dtn::data::Bundle> list1, list2;

	for (int i = 0; i < 2000; i++)
	{
		dtn::data::Bundle b;
		b._lifetime = 1;
		b._source = dtn::data::EID("dtn://node-two/foo");
		ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
		b.push_back(ref);

		(*ref.iostream()) << "Hallo Welt" << std::endl;

		if ((i % 3) == 0) list1.push_back(b); else list2.push_back(b);
	}

	// store list1 in the storage
	{ StoreProcess sp(storage, list1); sp.start(); }

	// store list1 in the storage
	{
		StoreProcess sp(storage, list2);
		sp.start();

		// in parallel we try to retrieve bundles from list1
		for (std::list<dtn::data::Bundle>::const_iterator iter = list1.begin(); iter != list1.end(); iter++)
		{
			const dtn::data::Bundle &b = (*iter);
			dtn::data::Bundle bundle = storage.get(b);
			storage.remove(bundle);
		}
	}

	storage.clear();

	// shutdown all services
	try {
		dtn::daemon::Component &component = dynamic_cast<dtn::daemon::Component&>(storage);
		component.terminate();
	} catch (const std::bad_cast&) { };

	core.terminate();
}

void SimpleBundleStorageTest::testDiskRestore()
{
	ibrcommon::File workdir("/tmp/bundle-disk-test");
	if (workdir.exists()) workdir.remove(true);
	ibrcommon::File::createDirectory(workdir);

	// create a persistent storage and insert some bundles
	{
		dtn::storage::SimpleBundleStorage storage(workdir);

		// startup all services
		storage.initialize();
		storage.startup();

		// create some bundles
		for (int i = 0; i < 2000; i++)
		{
			dtn::data::Bundle b;
			b._lifetime = 1;
			b._source = dtn::data::EID("dtn://node-two/foo");
			ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
			b.push_back(ref);

			(*ref.iostream()) << "Hallo Welt" << std::endl;

			storage.store(b);
		}

		// shutdown all services
		storage.terminate();
	}

	// Create a second storage in the same workdir. All bundles should be restored.
	{
		dtn::storage::SimpleBundleStorage storage(workdir);

		// startup all services
		storage.initialize();
		storage.startup();

		// the storage should contain 2000 bundles
		CPPUNIT_ASSERT_EQUAL((unsigned int)2000, storage.count());

		// delete all bundles
		storage.clear();

		// shutdown all services
		storage.terminate();
	}
}
