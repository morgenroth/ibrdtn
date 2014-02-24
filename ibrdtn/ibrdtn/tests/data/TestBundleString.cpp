/*
 * TestBundleString.cpp
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#include "data/TestBundleString.h"
#include <ibrdtn/data/EID.h>
#include <cppunit/extensions/HelperMacros.h>


CPPUNIT_TEST_SUITE_REGISTRATION (TestBundleString);

void TestBundleString::setUp(void)
{
}

void TestBundleString::tearDown(void)
{
}

void TestBundleString::mainTest(void)
{
	const dtn::data::BundleString data("Hello World");

	std::stringstream ss;

	ss << data;

	CPPUNIT_ASSERT_EQUAL(((const std::string&)data).length() + 1, ss.str().length());
}
