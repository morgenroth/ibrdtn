/*
 * TimerTest.cpp
 *
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

#include "thread/TimerTest.h"
#include <unistd.h>

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

	ibrcommon::Thread::sleep(2000);

	timer.stop();
	timer.join();
}
