/*
 * TestEID.cpp
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

#include "data/TestEID.h"
#include <ibrdtn/data/EID.h>
#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION (TestEID);

void TestEID::setUp(void)
{
}

void TestEID::tearDown(void)
{
}

void TestEID::testCBHEwithApplication(void)
{
	dtn::data::EID a("ipn:12.34");

	CPPUNIT_ASSERT(a.isCompressable());
	CPPUNIT_ASSERT_EQUAL((size_t)12, a.getCompressed().first.get<size_t>());
	CPPUNIT_ASSERT_EQUAL((size_t)34, a.getCompressed().second.get<size_t>());
	CPPUNIT_ASSERT_EQUAL(std::string("ipn:12.34"), a.getString());
}

void TestEID::testCBHEwithoutApplication(void)
{
	dtn::data::EID a("ipn:12.0");

	CPPUNIT_ASSERT(a.isCompressable());
	CPPUNIT_ASSERT_EQUAL((size_t)12, a.getCompressed().first.get<size_t>());
	CPPUNIT_ASSERT_EQUAL((size_t)0, a.getCompressed().second.get<size_t>());
	CPPUNIT_ASSERT_EQUAL(std::string("ipn:12.0"), a.getString());
}

void TestEID::testCBHEConstructorNumbers(void)
{
	dtn::data::EID a(12, 0);

	CPPUNIT_ASSERT(a.isCompressable());
	CPPUNIT_ASSERT_EQUAL((size_t)12, a.getCompressed().first.get<size_t>());
	CPPUNIT_ASSERT_EQUAL((size_t)0, a.getCompressed().second.get<size_t>());
	CPPUNIT_ASSERT_EQUAL(std::string("ipn:12.0"), a.getString());
}

void TestEID::testCBHEConstructorSchemeSsp(void)
{
	dtn::data::EID a("ipn", "12.34");

	CPPUNIT_ASSERT(a.isCompressable());
	CPPUNIT_ASSERT_EQUAL((size_t)12, a.getCompressed().first.get<size_t>());
	CPPUNIT_ASSERT_EQUAL((size_t)34, a.getCompressed().second.get<size_t>());
	CPPUNIT_ASSERT_EQUAL(std::string("ipn:12.34"), a.getString());
}

void TestEID::testCBHEEquals(void)
{
	dtn::data::EID a("ipn", "12.34");
	dtn::data::EID b("ipn", "12.34");

	CPPUNIT_ASSERT(a == b);
}

void TestEID::testCBHEHost(void)
{
	dtn::data::EID a("ipn:12.34");
	CPPUNIT_ASSERT_EQUAL(std::string("12"), a.getHost());
	CPPUNIT_ASSERT_EQUAL(std::string("ipn:12.0"), a.getNode().getString());
}
