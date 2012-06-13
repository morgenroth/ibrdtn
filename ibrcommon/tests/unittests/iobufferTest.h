/*
 * iobufferTest.h
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

#ifndef IOBUFFERTEST_H_
#define IOBUFFERTEST_H_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class iobufferTest : public CppUnit::TestFixture
{
public:
	void basicTest();

	void setUp();
	void tearDown();

	CPPUNIT_TEST_SUITE(iobufferTest);
	CPPUNIT_TEST(basicTest);
	CPPUNIT_TEST_SUITE_END();
};

#endif /* IOBUFFERTEST_H_ */
