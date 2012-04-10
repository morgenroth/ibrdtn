/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        ThreadTest.hh
/// @brief       CPPUnit-Tests for class Thread
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef THREADTEST_HH
#define THREADTEST_HH
class ThreadTest : public CppUnit::TestFixture {
	private:
	public:
		/*=== BEGIN tests for class 'Thread' ===*/
		void testYield();
		void testSleep();
		void testExit();
		void testDetach();
		void testConcurrency();
		void testReady();
		void testWaitready();
		void testEnableCancel();
		void testDisableCancel();
		void testRestoreCancel();
		void testCancel1();
		void testKill();
		void testCancel2();
		void testSetCancel();
		void testEqual();
		void testExec_thread();
		/*=== END   tests for class 'Thread' ===*/

		/*=== BEGIN tests for class 'JoinableThread' ===*/
		void testJoinableThreadJoin();
		void testJoinableThreadStart();
		void testJoinableThreadStop();
		/*=== END   tests for class 'JoinableThread' ===*/

		/*=== BEGIN tests for class 'DetachedThread' ===*/
		void testDetachedThreadExit();
		void testDetachedThreadStart();
		void testDetachedThreadStop();
		/*=== END   tests for class 'DetachedThread' ===*/

		void setUp();
		void tearDown();


		CPPUNIT_TEST_SUITE(ThreadTest);
			CPPUNIT_TEST(testYield);
			CPPUNIT_TEST(testSleep);
			CPPUNIT_TEST(testExit);
			CPPUNIT_TEST(testDetach);
			CPPUNIT_TEST(testConcurrency);
			CPPUNIT_TEST(testReady);
			CPPUNIT_TEST(testWaitready);
			CPPUNIT_TEST(testEnableCancel);
			CPPUNIT_TEST(testDisableCancel);
			CPPUNIT_TEST(testRestoreCancel);
			CPPUNIT_TEST(testCancel1);
			CPPUNIT_TEST(testKill);
			CPPUNIT_TEST(testCancel2);
			CPPUNIT_TEST(testSetCancel);
			CPPUNIT_TEST(testEqual);
			CPPUNIT_TEST(testExec_thread);
			CPPUNIT_TEST(testJoinableThreadJoin);
			CPPUNIT_TEST(testJoinableThreadStart);
			CPPUNIT_TEST(testJoinableThreadStop);
			CPPUNIT_TEST(testDetachedThreadExit);
			CPPUNIT_TEST(testDetachedThreadStart);
			CPPUNIT_TEST(testDetachedThreadStop);
		CPPUNIT_TEST_SUITE_END();
};
#endif /* THREADTEST_HH */
