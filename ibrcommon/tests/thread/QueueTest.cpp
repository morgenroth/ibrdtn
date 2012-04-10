/*
 * QueueTest.cpp
 *
 *  Created on: 22.09.2010
 *      Author: morgenro
 */

#include "thread/QueueTest.h"
#include <stdlib.h>
#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION (QueueTest);

void QueueTest::setUp()
{
}

void QueueTest::tearDown()
{
}

QueueTest::TestThread::TestThread(size_t time, size_t max)
 : _count(0), _time(time), _max(max)
{
}

QueueTest::TestThread::~TestThread()
{
	join();
}

void QueueTest::TestThread::run()
{
	try {
		while (true)
		{
			std::string data = _queue.getnpop(true);
			_count++;

			if (_time > 0) ::usleep(_time);

			if ((_max > 0) && (_max <= _count)) return;
		}
	} catch (const std::exception&) {

	}
}

void QueueTest::TestThread::__cancellation()
{
	_queue.abort();
}

void QueueTest::TestThread::finally()
{
}

void QueueTest::tsq_test01()
{
	TestThread t;
	t.start();

	::usleep(1000);

	t.stop();
	t.join();
}

void QueueTest::tsq_test02()
{
	TestThread t(0, 2);
	t.start();

	t._queue.push("hallo");
	t._queue.push("welt");

	t.join();

	CPPUNIT_ASSERT_EQUAL((size_t)2, t._count);
}

void QueueTest::tsq_test03()
{
	TestThread t;
	t.start();

	t._queue.push("hallo");
	t._queue.push("welt");

	::sleep(1);

	t._queue.abort();
	t.join();

	CPPUNIT_ASSERT_EQUAL((size_t)2, t._count);
}

void QueueTest::tsq_test04()
{
	TestThread t(2000);
	t.start();

	t._queue.push("hallo");
	t._queue.push("welt");

	::usleep(1000);

	t._queue.abort();
	t.join();

	CPPUNIT_ASSERT_EQUAL((size_t)2, t._count);
}

void QueueTest::tsq_test05()
{
	for (int i = 0; i < 100; i++)
	{
		TestThread t(200 / (i+1));
		t.start();

		for (int j = 0; j < i; j++)
		{
			t._queue.push("hallo");
			t._queue.push("welt");
		}

		::usleep(i);

		t._queue.abort();
		t.join();

		//CPPUNIT_ASSERT_EQUAL((size_t)2, t._count);
	}
}
