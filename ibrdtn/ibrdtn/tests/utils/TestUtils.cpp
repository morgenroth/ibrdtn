/*
 * TestUtils.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Stephen Roettger <roettger@ibr.cs.tu-bs.de>
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

#include "TestUtils.h"

#include <ibrdtn/utils/Utils.h>

CPPUNIT_TEST_SUITE_REGISTRATION (TestUtils);

void TestUtils::setUp()
{
}

void TestUtils::tearDown()
{
}

void TestUtils::tokenizeEmptyTest()
{
	using namespace dtn::utils;

	// empty test1
	CPPUNIT_ASSERT(Utils::tokenize(":", "").empty());

	// empty test2
	CPPUNIT_ASSERT(Utils::tokenize(":", "::").empty());
}

void TestUtils::tokenizeSizeTest()
{
	using namespace dtn::utils;

	CPPUNIT_ASSERT_EQUAL((size_t)2, Utils::tokenize(":", ":a:test::", 2).size());
	CPPUNIT_ASSERT_EQUAL((size_t)3, Utils::tokenize(":", ":a:test::b::", 2).size());
}

void TestUtils::tokenizeSizeSpacesTest()
{
	using namespace dtn::utils;

	CPPUNIT_ASSERT_EQUAL((size_t)1, Utils::tokenize(":", ": :", 1).size());
	CPPUNIT_ASSERT_EQUAL((size_t)3, Utils::tokenize(":", ":    :t e s t: ").size());
}

void TestUtils::tokenizeLastItemTest()
{
	using namespace dtn::utils;

	//TODO how should the added string in the last item look like? "b::" or ":b::" or "::b::"
	CPPUNIT_ASSERT_EQUAL(std::string("b::"), Utils::tokenize(":", ":a:test::b::", 2)[2]);
}

