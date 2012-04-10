/*
 * MutexTests.h
 *
 *  Created on: 16.04.2010
 *      Author: morgenro
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef MUTEXTESTS_H_
#define MUTEXTESTS_H_

#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/thread/Thread.h>

class MutexTests : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (MutexTests);
	CPPUNIT_TEST (mutex_test01);
	CPPUNIT_TEST (mutex_trylock);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	class TestThread : public ibrcommon::JoinableThread
	{
	public:
		TestThread(ibrcommon::Mutex &m, bool &var, const bool value);
		~TestThread();

		void run();
		void __cancellation();

	private:
		ibrcommon::Mutex &_m;
		bool &_var;
		bool _value;
	};

	void mutex_test01();
	void mutex_trylock();
};

#endif /* MUTEXTESTS_H_ */
