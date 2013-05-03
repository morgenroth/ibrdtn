/*
 * DatagramClTest.cpp
 *
 *  Created on: 03.05.2013
 *      Author: morgenro
 */

#include "DatagramClTest.h"
#include "../tools/TestEventListener.h"
#include "storage/MemoryBundleStorage.h"
#include "core/NodeEvent.h"

#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>
#include <ibrcommon/thread/Thread.h>
#include "core/TimeEvent.h"
#include <ibrdtn/utils/Clock.h>
#include "core/BundleCore.h"
#include <ibrcommon/data/File.h>
#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrdtn/data/AgeBlock.h>
#include "Component.h"

#include <unistd.h>

CPPUNIT_TEST_SUITE_REGISTRATION(DatagramClTest);

dtn::storage::BundleStorage* DatagramClTest::_storage = NULL;

void DatagramClTest::setUp() {
	// create a new event switch
	_esl = new ibrtest::EventSwitchLoop();

	// enable blob path
	ibrcommon::File blob_path("/tmp/blobs");

	// check if the BLOB path exists
	if (!blob_path.exists()) {
		// try to create the BLOB path
		ibrcommon::File::createDirectory(blob_path);
	}

	// enable the blob provider
	ibrcommon::BLOB::changeProvider(new ibrcommon::FileBLOBProvider(blob_path), true);

	// add standard memory base storage
	_storage = new dtn::storage::MemoryBundleStorage();

	// create fake datagram service
	_fake_service = new FakeDatagramService();
	_fake_cl = new DatagramConvergenceLayer( _fake_service );

	// initialize BundleCore
	dtn::core::BundleCore::getInstance().initialize();

	// start-up event switch
	_esl->start();

	_fake_cl->initialize();

	try {
		dtn::daemon::Component &c = dynamic_cast<dtn::daemon::Component&>(*_storage);
		c.initialize();
	} catch (const bad_cast&) {
	}

	// startup BundleCore
	dtn::core::BundleCore::getInstance().startup();

	_fake_cl->startup();

	try {
		dtn::daemon::Component &c = dynamic_cast<dtn::daemon::Component&>(*_storage);
		c.startup();
	} catch (const bad_cast&) {
	}
}

void DatagramClTest::tearDown() {
	_esl->stop();

	_fake_cl->terminate();

	try {
		dtn::daemon::Component &c = dynamic_cast<dtn::daemon::Component&>(*_storage);
		c.terminate();
	} catch (const bad_cast&) {
	}

	// shutdown BundleCore
	dtn::core::BundleCore::getInstance().terminate();

	delete _fake_cl;
	_fake_cl = NULL;

	_esl->join();
	delete _esl;
	_esl = NULL;

	// delete storage
	delete _storage;
}

void DatagramClTest::discoveryTest() {
	TestEventListener<dtn::core::NodeEvent> evtl;

	const std::set<dtn::core::Node> pre_disco_nodes = dtn::core::BundleCore::getInstance().getConnectionManager().getNeighbors();
	CPPUNIT_ASSERT_EQUAL((size_t)0, pre_disco_nodes.size());

	// send fake discovery beacon
	_fake_service->fakeDiscovery();

	// wait until the beacon has been processes
	ibrcommon::MutexLock l(evtl.event_cond);
	while (evtl.event_counter == 0) evtl.event_cond.wait();

	const std::set<dtn::core::Node> post_disco_nodes = dtn::core::BundleCore::getInstance().getConnectionManager().getNeighbors();
	CPPUNIT_ASSERT_EQUAL((size_t)1, post_disco_nodes.size());
}
