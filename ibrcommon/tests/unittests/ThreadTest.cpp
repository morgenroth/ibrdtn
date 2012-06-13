/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        ThreadTest.cpp
/// @brief       CPPUnit-Tests for class Thread
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

/*
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ThreadTest.hh"



CPPUNIT_TEST_SUITE_REGISTRATION(ThreadTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'Thread' ===*/
void ThreadTest::testYield()
{
	/* test signature (void) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testSleep()
{
	/* test signature (size_t timeout) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testExit()
{
	/* test signature (void) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testDetach()
{
	/* test signature (void) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testConcurrency()
{
	/* test signature (int level) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testReady()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testWaitready()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testEnableCancel()
{
	/* test signature (int &state) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testDisableCancel()
{
	/* test signature (int &state) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testRestoreCancel()
{
	/* test signature (const int &state) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testCancel1()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testKill()
{
	/* test signature (int sig) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testCancel2()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testSetCancel()
{
	/* test signature (bool val) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testEqual()
{
	/* test signature (pthread_t thread1, pthread_t thread2) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testExec_thread()
{
	/* test signature (void *obj) */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'Thread' ===*/

/*=== BEGIN tests for class 'JoinableThread' ===*/
void ThreadTest::testJoinableThreadJoin()
{
	/* test signature (void) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testJoinableThreadStart()
{
	/* test signature (int priority = 0) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testJoinableThreadStop()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'JoinableThread' ===*/

/*=== BEGIN tests for class 'DetachedThread' ===*/
void ThreadTest::testDetachedThreadExit()
{
	/* test signature (void) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testDetachedThreadStart()
{
	/* test signature (int priority = 0) */
	CPPUNIT_FAIL("not implemented");
}

void ThreadTest::testDetachedThreadStop()
{
	/* test signature () */
	CPPUNIT_FAIL("not implemented");
}

/*=== END   tests for class 'DetachedThread' ===*/

void ThreadTest::setUp()
{
}

void ThreadTest::tearDown()
{
}

