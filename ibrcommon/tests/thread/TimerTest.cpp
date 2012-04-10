/*
 * TimerTest.cpp
 *
 *  Created on: 14.09.2010
 *      Author: morgenro
 */

#include "thread/TimerTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION (TimerTest);

void TimerTest::setUp()
{
}

void TimerTest::tearDown()
{
}

size_t TimerTest::timeout(ibrcommon::Timer*)
{
	return 60;
}

void TimerTest::timer_test01()
{
	ibrcommon::Timer timer(*this, 0);

	timer.set(60);
	timer.start();

	::sleep(2);

	timer.stop();
	timer.join();
}
