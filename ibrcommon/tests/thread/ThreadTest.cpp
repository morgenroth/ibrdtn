/*
 * ThreadTest.cpp
 *
 *  Created on: 11.09.2010
 *      Author: jm
 */

#include "thread/ThreadTest.h"
#include <stdlib.h>
#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION (ThreadTest);

void ThreadTest::setUp()
{
}

void ThreadTest::tearDown()
{
}

ThreadTest::DetachedTestThread::DetachedTestThread(size_t &finally, bool &run, size_t time)
 : _finally(finally), _run(run), _time(time)
{

}

ThreadTest::DetachedTestThread::~DetachedTestThread()
{

}

void ThreadTest::DetachedTestThread::run()
{
	_run = true;
	::usleep(_time);
}

void ThreadTest::DetachedTestThread::finally()
{
	_finally++;
}

void ThreadTest::DetachedTestThread::__cancellation()
{
}

ThreadTest::TestThread::TestThread(size_t time)
 : _finally(0), _time(time)
{
}

ThreadTest::TestThread::~TestThread()
{
	join();
}

void ThreadTest::TestThread::__cancellation()
{
}

void ThreadTest::TestThread::run()
{
	if (_time == 0)
	{
		while (true) ::usleep(1000);
	}
	else
	{
		for (size_t i = 0; i < _time; i++)
			::usleep(1000);
	}
}

void ThreadTest::TestThread::finally()
{
	_finally++;
}

void ThreadTest::thread_test01()
{
	TestThread t;
	t.start();

	::usleep(1000);

	t.stop();
	t.join();

	CPPUNIT_ASSERT_EQUAL(1, (int)t._finally);
}

void ThreadTest::thread_test02()
{
	TestThread t(2000);
	t.start();

	::usleep(1000);

	t.join();

	CPPUNIT_ASSERT_EQUAL(1, (int)t._finally);
}

void ThreadTest::thread_test03()
{
	bool run = false;
	size_t finally = 0;

	DetachedTestThread *t = new DetachedTestThread(finally, run, 10);
	t->start();

	::usleep(2000);

	CPPUNIT_ASSERT_EQUAL(1, (int)finally);
	CPPUNIT_ASSERT_EQUAL(true, run);
}

void ThreadTest::thread_test04()
{
	bool run = false;
	size_t finally = 0;

	DetachedTestThread *t = new DetachedTestThread(finally, run, 1000);
	t->start();

	::usleep(100);
	t->stop();

	::usleep(2000);

	CPPUNIT_ASSERT_EQUAL(1, (int)finally);
	CPPUNIT_ASSERT_EQUAL(true, run);
}
