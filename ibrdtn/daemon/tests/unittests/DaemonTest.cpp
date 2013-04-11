#include "DaemonTest.hh"
#include "NativeDaemon.h"
#include <ibrcommon/Logger.h>

CPPUNIT_TEST_SUITE_REGISTRATION(DaemonTest);
/*========================== tests below ==========================*/

/*=== BEGIN tests for dtnd ===*/

void DaemonTest::testUpDownUp(){

	dtn::daemon::NativeDaemon dtnd;

	CPPUNIT_ASSERT_NO_THROW(dtnd.init(dtn::daemon::RUNLEVEL_ROUTING_EXTENSIONS));
	CPPUNIT_ASSERT_NO_THROW(dtnd.init(dtn::daemon::RUNLEVEL_ZERO));

	CPPUNIT_ASSERT_NO_THROW(dtnd.init(dtn::daemon::RUNLEVEL_ROUTING_EXTENSIONS));
	CPPUNIT_ASSERT_NO_THROW(dtnd.init(dtn::daemon::RUNLEVEL_ZERO));
}


void DaemonTest::setUp()
{
}

void DaemonTest::tearDown()
{
}

