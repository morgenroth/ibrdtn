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

#include <ibrdtn/data/Number.h>
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
	CPPUNIT_ASSERT_EQUAL((size_t)1, dtn::data::Number(127).getLength());
	CPPUNIT_ASSERT_EQUAL((size_t)2, dtn::data::Number(700).getLength());
	CPPUNIT_ASSERT_EQUAL((size_t)3, dtn::data::Number(32896).getLength());
	CPPUNIT_ASSERT_EQUAL((size_t)4, dtn::data::Number(8388608).getLength());
}

void TestSDNV::testSerialize(void)
{
	for (uint64_t i = 0; i <= (2^32); i += (2^7))
	{
		std::stringstream ss;
		dtn::data::Number src(i);
		dtn::data::Number dst;

		ss << src;
		ss.clear();
		ss >> dst;

		CPPUNIT_ASSERT_EQUAL(ss.gcount(), (std::streamsize)src.getLength());
		CPPUNIT_ASSERT_EQUAL(src.get<size_t>(), dst.get<size_t>());
	}
}

void TestSDNV::testSizeT(void)
{
	size_t st = 4096;

	std::stringstream ss;
	dtn::data::Number src(st);
	dtn::data::Number dst;

	ss << src;
	ss.clear();
	ss >> dst;

	CPPUNIT_ASSERT_EQUAL(st, dst.get<size_t>());
}

void TestSDNV::testMax(void)
{
	std::stringstream ss;
	dtn::data::Number src = dtn::data::Number::max();
	dtn::data::Number dst;

	CPPUNIT_ASSERT_NO_THROW( ss << src );
	ss.clear();
	CPPUNIT_ASSERT_NO_THROW( ss >> dst );

	CPPUNIT_ASSERT_EQUAL(src, dst);
}

void TestSDNV::testMax32(void)
{
	std::stringstream ss;
	dtn::data::SDNV<uint32_t> src = dtn::data::SDNV<uint32_t>::max();
	dtn::data::SDNV<uint32_t> dst;

	CPPUNIT_ASSERT_NO_THROW( ss << src );
	ss.clear();
	CPPUNIT_ASSERT_NO_THROW( ss >> dst );

	CPPUNIT_ASSERT_EQUAL(src, dst);
}

void TestSDNV::testTrim(void)
{
	dtn::data::SDNV<uint64_t> number = dtn::data::SDNV<uint32_t>::max().get<uint64_t>() + 1000;

	CPPUNIT_ASSERT( number > dtn::data::SDNV<uint32_t>::max().get<uint64_t>() );

	number.trim<uint32_t>();

	CPPUNIT_ASSERT( number <= dtn::data::SDNV<uint32_t>::max().get<uint64_t>() );
}

void TestSDNV::testOutOfRange(void)
{
	uint64_t st = static_cast<uint64_t>(-1);

	std::stringstream ss;
	dtn::data::SDNV<uint64_t> src(st);
	dtn::data::SDNV<uint32_t> dst;

	CPPUNIT_ASSERT_NO_THROW( ss << src );
	ss.clear();
	CPPUNIT_ASSERT_THROW( ss >> dst, dtn::data::ValueOutOfRangeException );
}

enum FLAGS {
	HIGHBIT = (size_t)1 << 0x1F
};

void TestSDNV::testBitset(void)
{
	dtn::data::Bitset<FLAGS> bs;

	bs.setBit(HIGHBIT, true);

	size_t value = 0x80000000;
	CPPUNIT_ASSERT_EQUAL(value, bs.get<size_t>());

	std::stringstream ss;
	dtn::data::SDNV<uint32_t> dst;
	CPPUNIT_ASSERT_NO_THROW( ss >> dst );
}
