/*
 * TestUtils.cpp
 *
 *  Created on: 30.04.2012
 *      Author: roettger
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

void TestUtils::tokenizeTest()
{
	using namespace dtn::utils;
	CPPUNIT_ASSERT(Utils::tokenize(":", "").empty());
	CPPUNIT_ASSERT(Utils::tokenize(":", "::").empty());
	CPPUNIT_ASSERT_EQUAL((int)Utils::tokenize(":", ":a:test::", 2).size(), 2);
	CPPUNIT_ASSERT_EQUAL((int)Utils::tokenize(":", ":a:test::b::", 2).size(), 3);
	//TODO how should the added string in the last item look like? "b::" or ":b::" or "::b::"
	CPPUNIT_ASSERT(Utils::tokenize(":", ":a:test::b::", 2)[2] == "b::");
	CPPUNIT_ASSERT_EQUAL((int)Utils::tokenize(":", ": :", 1).size(), 1);
	CPPUNIT_ASSERT_EQUAL((int)Utils::tokenize(":", ":    :t e s t: ").size(), 3);
}

