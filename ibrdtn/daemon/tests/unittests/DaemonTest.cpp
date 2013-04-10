#include "DaemonTest.hh"
#include "ibrdtnd.h"
#include <ibrcommon/Logger.h>

CPPUNIT_TEST_SUITE_REGISTRATION(DaemonTest);
/*========================== tests below ==========================*/

/*=== BEGIN tests for dtnd ===*/

void DaemonTest::testUpDownUp(){

	int initReturn_1 = -1;
	int loopReturn_1 = -1;
	int shutReturn_1 = -1;

	int initReturn_2 = -1;
	int loopReturn_2 = -1;
	int shutReturn_2 = -1;

	initReturn_1 = ibrdtn_daemon_initialize();
	CPPUNIT_ASSERT_EQUAL(initReturn_1,0);

	shutReturn_1 = ibrdtn_daemon_shutdown();
	CPPUNIT_ASSERT_EQUAL(shutReturn_1,0);

	loopReturn_1 = ibrdtn_daemon_main_loop();
	CPPUNIT_ASSERT_EQUAL(loopReturn_1,0);


	initReturn_2 = ibrdtn_daemon_initialize();
	CPPUNIT_ASSERT_EQUAL(initReturn_2,0);

	shutReturn_2 = ibrdtn_daemon_shutdown();
        CPPUNIT_ASSERT_EQUAL(shutReturn_2,0);

	loopReturn_2 = ibrdtn_daemon_main_loop();
	CPPUNIT_ASSERT_EQUAL(loopReturn_2,0);
}


void DaemonTest::setUp()
{
}

void DaemonTest::tearDown()
{
}

