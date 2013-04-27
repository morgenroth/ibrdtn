/*
 * TestSDNV.cpp
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

#include "data/TestSDNV.h"
#include <cppunit/extensions/HelperMacros.h>

#include <ibrdtn/data/SDNV.h>
#include <sstream>
#include <stdint.h>

CPPUNIT_TEST_SUITE_REGISTRATION (TestSDNV);

void TestSDNV::hexdump(char c) {
  std::cout << std::hex << (int)c << " ";
}

void TestSDNV::setUp(void)
{
}

void TestSDNV::tearDown(void)
{
}

void TestSDNV::testLength(void)
{
	CPPUNIT_ASSERT_EQUAL((size_t)1, dtn::data::SDNV(127).getLength());
	CPPUNIT_ASSERT_EQUAL((size_t)2, dtn::data::SDNV(700).getLength());
	CPPUNIT_ASSERT_EQUAL((size_t)3, dtn::data::SDNV(32896).getLength());
	CPPUNIT_ASSERT_EQUAL((size_t)4, dtn::data::SDNV(8388608).getLength());
}

void TestSDNV::testSerialize(void)
{
	for (uint64_t i = 0; i <= (2^32); i += (2^7))
	{
		std::stringstream ss;
		dtn::data::SDNV src(i);
		dtn::data::SDNV dst;

		ss << src;
		ss.clear();
		ss >> dst;

		CPPUNIT_ASSERT_EQUAL((size_t)ss.gcount(), src.getLength());
		CPPUNIT_ASSERT_EQUAL(src.getValue(), dst.getValue());
	}
}

void TestSDNV::testSizeT(void)
{
	size_t st = 4096;

	std::stringstream ss;
	dtn::data::SDNV src(st);
	dtn::data::SDNV dst;

	ss << src;
	ss.clear();
	ss >> dst;

	CPPUNIT_ASSERT_EQUAL(st, (size_t)dst.getValue());
}

void TestSDNV::testMax(void)
{
	uint64_t st = static_cast<uint64_t>(-1);

	std::stringstream ss;
	dtn::data::SDNV src(st);
	dtn::data::SDNV dst;

	CPPUNIT_ASSERT_NO_THROW( ss << src );
	ss.clear();
	CPPUNIT_ASSERT_NO_THROW( ss >> dst );

	CPPUNIT_ASSERT_EQUAL(st, dst.getValue());
}
