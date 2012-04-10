/*
 * TimerTest.h
 *
 *  Created on: 14.09.2010
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TIMERTEST_H_
#define TIMERTEST_H_

#include <ibrcommon/thread/Timer.h>

class TimerTest : public CPPUNIT_NS :: TestFixture, public ibrcommon::TimerCallback
{
	CPPUNIT_TEST_SUITE (TimerTest);
	CPPUNIT_TEST (timer_test01);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp (void);
	void tearDown (void);

	size_t timeout(ibrcommon::Timer *timer);

protected:
	void timer_test01();
};

#endif /* TIMERTEST_H_ */
