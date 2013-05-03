/*
 * DatagramClTest.h
 *
 *  Created on: 03.05.2013
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "storage/BundleStorage.h"
#include "net/DatagramConvergenceLayer.h"
#include "net/DatagramService.h"

#include "FakeDatagramService.h"
#include "../tools/EventSwitchLoop.h"

#ifndef DATAGRAMCLTEST_H_
#define DATAGRAMCLTEST_H_

class DatagramClTest : public CppUnit::TestFixture {
	static dtn::storage::BundleStorage *_storage;
	ibrtest::EventSwitchLoop *_esl;
	FakeDatagramService *_fake_service;
	dtn::net::DatagramConvergenceLayer *_fake_cl;

	void discoveryTest();
	void queueTest();

public:
	void setUp();
	void tearDown();

	CPPUNIT_TEST_SUITE(DatagramClTest);
	CPPUNIT_TEST(discoveryTest);
	CPPUNIT_TEST(queueTest);
	CPPUNIT_TEST_SUITE_END();
};

#endif /* DATAGRAMCLTEST_H_ */
