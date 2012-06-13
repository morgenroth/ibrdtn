/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BaseRouterTest.cpp
/// @brief       CPPUnit-Tests for class BaseRouter
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

#include "BaseRouterTest.hh"
#include "src/routing/BaseRouter.h"
#include "src/storage/BundleStorage.h"
#include "src/core/Node.h"
#include "tests/tools/EventSwitchLoop.h"
#include "src/net/BundleReceivedEvent.h"
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>
#include <ibrcommon/thread/Thread.h>
#include <ibrcommon/Logger.h>


CPPUNIT_TEST_SUITE_REGISTRATION(BaseRouterTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'BaseRouter' ===*/
/*=== BEGIN tests for class 'Extension' ===*/
void BaseRouterTest::testGetRouter()
{
	class ExtensionTest : public dtn::routing::BaseRouter::Extension
	{
	public:
		ExtensionTest() {};
		~ExtensionTest() {};

		void notify(const dtn::core::Event*) {};

		dtn::routing::BaseRouter& testGetRouter()
		{
			return **this;
		};
	};

	dtn::routing::BaseRouter router(_storage);
	ExtensionTest e;
	CPPUNIT_ASSERT_EQUAL(&router, &e.testGetRouter());
}

/*=== END   tests for class 'Extension' ===*/

void BaseRouterTest::testAddExtension()
{
	/* test signature (BaseRouter::Extension *extension) */
	class ExtensionTest : public dtn::routing::BaseRouter::Extension
	{
	public:
		ExtensionTest() {};
		~ExtensionTest() {};

		void notify(const dtn::core::Event*) {};

		dtn::routing::BaseRouter& testGetRouter()
		{
			return **this;
		};
	};

	dtn::routing::BaseRouter router(_storage);
	ExtensionTest *e = new ExtensionTest();
	router.addExtension(e);
}

void BaseRouterTest::testTransferTo()
{
	/* test signature (BaseRouter::Extension *extension) */
	class ExtensionTest : public dtn::routing::BaseRouter::Extension
	{
	public:
		ExtensionTest() {};
		~ExtensionTest() {};

		void notify(const dtn::core::Event*) {};
	};

	/* test signature (const dtn::data::EID &destination, const dtn::data::BundleID &id) */
	dtn::data::Bundle b;
	dtn::routing::BaseRouter router(_storage);
	dtn::routing::NeighborDatabase::NeighborEntry n(dtn::data::EID("dtn://no-neighbor"));
	ExtensionTest *ex = new ExtensionTest();
	router.addExtension(ex);
	ex->transferTo(n, b);
}

void BaseRouterTest::testRaiseEvent()
{
	/* test signature (const dtn::core::Event *evt) */
	dtn::data::EID eid("dtn://no-neighbor");
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://testcase-one/foo");

	ibrtest::EventSwitchLoop esl; esl.start();
	dtn::routing::BaseRouter router(_storage);
	router.initialize();

	CPPUNIT_ASSERT_EQUAL(false, router.isKnown(b));

	// send a bundle
	dtn::net::BundleReceivedEvent::raise(eid, b, false, true);

	// this bundle has to be known in future
	CPPUNIT_ASSERT_EQUAL(true, router.isKnown(b));

	dtn::core::GlobalEvent::raise(dtn::core::GlobalEvent::GLOBAL_SHUTDOWN);
	esl.join();

	router.terminate();
}

void BaseRouterTest::testGetBundle()
{
	/* test signature (const dtn::data::BundleID &id) */
	dtn::routing::BaseRouter router(_storage);

	dtn::data::Bundle b1;
	_storage.store(b1);

	try {
		dtn::data::Bundle b2 = router.getBundle(b1);
	} catch (const ibrcommon::Exception&) {
		CPPUNIT_FAIL("no bundle returned");
	}
}

void BaseRouterTest::testGetStorage()
{
	/* test signature () */
	dtn::routing::BaseRouter router(_storage);
	CPPUNIT_ASSERT_EQUAL((dtn::storage::BundleStorage*)&_storage, &router.getStorage());
}

void BaseRouterTest::testIsKnown()
{
	/* test signature (const dtn::data::BundleID &id) */
	dtn::routing::BaseRouter router(_storage);

	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://testcase-one/foo");

	CPPUNIT_ASSERT_EQUAL(false, router.isKnown(b));

	router.setKnown(b);

	CPPUNIT_ASSERT_EQUAL(true, router.isKnown(b));
}

void BaseRouterTest::testSetKnown()
{
	/* test signature (const dtn::data::MetaBundle &meta) */
	dtn::routing::BaseRouter router(_storage);

	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://testcase-one/foo");

	router.setKnown(b);
	CPPUNIT_ASSERT_EQUAL(true, router.isKnown(b));
}

void BaseRouterTest::testGetSummaryVector()
{
	/* test signature () */
	dtn::routing::BaseRouter router(_storage);

	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://testcase-one/foo");

	CPPUNIT_ASSERT_EQUAL(false, router.isKnown(b));

	router.setKnown(b);

	CPPUNIT_ASSERT_EQUAL(true, router.getSummaryVector().contains(b));
}

/*=== END   tests for class 'BaseRouter' ===*/

void BaseRouterTest::setUp()
{
	_storage.clear();
}

void BaseRouterTest::tearDown()
{
}

