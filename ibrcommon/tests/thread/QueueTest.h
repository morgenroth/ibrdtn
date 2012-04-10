/*
 * QueueTest.h
 *
 *  Created on: 22.09.2010
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef IBRCOMMON_QUEUETEST_H_
#define IBRCOMMON_QUEUETEST_H_

#include <ibrcommon/thread/Queue.h>
#include <ibrcommon/thread/Thread.h>

class QueueTest : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (QueueTest);
	CPPUNIT_TEST (tsq_test01);
	CPPUNIT_TEST (tsq_test02);
	CPPUNIT_TEST (tsq_test03);
	CPPUNIT_TEST (tsq_test04);
	CPPUNIT_TEST (tsq_test05);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp (void);
	void tearDown (void);

	class TestThread : public ibrcommon::JoinableThread
	{
	public:
		TestThread(size_t time = 0, size_t max = 0);
		~TestThread();

		void run();
		virtual void __cancellation();

		void finally();

		size_t _count;
		size_t _time;
		size_t _max;

		ibrcommon::Queue<std::string> _queue;
	};

protected:
	void tsq_test01();
	void tsq_test02();
	void tsq_test03();
	void tsq_test04();
	void tsq_test05();
};

#endif /* IBRCOMMON_QUEUETEST_H_ */
