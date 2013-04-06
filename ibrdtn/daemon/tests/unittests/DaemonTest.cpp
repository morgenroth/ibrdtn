#include "DaemonTest.hh"
#include "NativeDaemon.h"
#include <ibrcommon/Logger.h>

CPPUNIT_TEST_SUITE_REGISTRATION(DaemonTest);
/*========================== tests below ==========================*/

/*=== BEGIN tests for dtnd ===*/

void DaemonTest::testUpDownUp(){

	dtn::daemon::NativeDaemon dtnd;

	CPPUNIT_ASSERT_NO_THROW(dtnd.initialize());
	dtnd.shutdown();
	CPPUNIT_ASSERT_NO_THROW(dtnd.main_loop());

	CPPUNIT_ASSERT_NO_THROW(dtnd.initialize());
	dtnd.shutdown();
	CPPUNIT_ASSERT_NO_THROW(dtnd.main_loop());
}


void DaemonTest::setUp()
{
}

void DaemonTest::tearDown()
{
}

