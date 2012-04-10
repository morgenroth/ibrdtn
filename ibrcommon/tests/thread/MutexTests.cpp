/*
 * MutexTests.cpp
 *
 *  Created on: 16.04.2010
 *      Author: morgenro
 */

#include "thread/MutexTests.h"
#include <stdlib.h>
#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION (MutexTests);

void MutexTests::setUp()
{
}

void MutexTests::tearDown()
{
}

MutexTests::TestThread::TestThread(ibrcommon::Mutex &m, bool &var, const bool value)
 : _m(m), _var(var), _value(value)
{
}

MutexTests::TestThread::~TestThread()
{
	join();
}

void MutexTests::TestThread::run()
{
	ibrcommon::MutexLock l(_m);
	_var = _value;
}

void MutexTests::TestThread::__cancellation()
{
}

void MutexTests::mutex_trylock()
{
	ibrcommon::Mutex m;
	ibrcommon::MutexLock l1(m);

	try {
		ibrcommon::MutexTryLock l2(m);

		// we should never be here!
		CPPUNIT_ASSERT(false);
	} catch (const ibrcommon::MutexException&) {
		// we should land here
	}
}

void MutexTests::mutex_test01()
{
	ibrcommon::Mutex m1;
	ibrcommon::Mutex m2;
	bool var = false;

	m1.enter();
	TestThread t(m1, var, true);
	t.start();
	CPPUNIT_ASSERT(!var);

	m1.leave();

	::sleep(2);

	CPPUNIT_ASSERT(var);

	t.join();
}
