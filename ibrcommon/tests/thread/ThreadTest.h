/*
 * ThreadTest.h
 *
 *  Created on: 11.09.2010
 *      Author: jm
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef THREADTEST_H_
#define THREADTEST_H_

#include <ibrcommon/thread/Thread.h>

class ThreadTest : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (ThreadTest);
//	CPPUNIT_TEST (thread_test01);
	CPPUNIT_TEST (thread_test02);
	CPPUNIT_TEST (thread_test03);
//	CPPUNIT_TEST (thread_test04);
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp (void);
	void tearDown (void);

	class TestThread : public ibrcommon::JoinableThread
	{
	public:
		TestThread(size_t time = 0);
		~TestThread();

		void run();
		void __cancellation();
		void finally();

		size_t _finally;
		size_t _time;
	};

	class DetachedTestThread : public ibrcommon::DetachedThread
	{
	public:
		DetachedTestThread(size_t &finally, bool &run, size_t time = 0);
		~DetachedTestThread();

		void run();
		void __cancellation();
		void finally();

	private:
		size_t &_finally;
		bool &_run;
		size_t _time;
	};

protected:
	void thread_test01();
	void thread_test02();
	void thread_test03();
	void thread_test04();
};

#endif /* THREADTEST_H_ */
