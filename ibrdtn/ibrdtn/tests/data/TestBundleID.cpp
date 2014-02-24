/*
 * TestBundleID.cpp
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

#include "data/TestBundleID.h"
#include <ibrdtn/data/BundleID.h>
#include <ibrcommon/data/BloomFilter.h>
#include <ibrcommon/TimeMeasurement.h>
#include <string.h>
#include <iomanip>

CPPUNIT_TEST_SUITE_REGISTRATION (TestBundleID);

void TestBundleID::setUp()
{
}

void TestBundleID::tearDown()
{
}

void TestBundleID::rawTest(void)
{
	dtn::data::Bundle b;
	b.source = dtn::data::EID("dtn:test");
	b.timestamp = 1;
	b.sequencenumber = 1;

	dtn::data::MetaBundle m = dtn::data::MetaBundle::create(b);

	unsigned char b_data[2000];
	const size_t b_data_len = b.raw((unsigned char*)&b_data, 2000);

	unsigned char m_data[2000];
	m.fragmentoffset = 0;
	const size_t m_data_len = m.raw((unsigned char*)&m_data, 2000);

	CPPUNIT_ASSERT_EQUAL(b_data_len, m_data_len);

//	std::cout << std::endl;
//	for (size_t i = 0; i < b_data_len; ++i) {
//		std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)b_data[i];
//	}
//	std::cout << std::endl;
//	for (size_t i = 0; i < m_data_len; ++i) {
//		std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)m_data[i];
//	}
//	std::cout << std::endl;

	CPPUNIT_ASSERT_EQUAL(0, ::memcmp(b_data, m_data, b_data_len));
}

void TestBundleID::performanceTest(void)
{
	ibrcommon::TimeMeasurement tm;
	int i = 0;

	ibrcommon::BloomFilter bf;
	dtn::data::Bundle b;
	b.setFragment(true);
	b.fragmentoffset = 1023;

	const dtn::data::BundleID id = b;

	tm.start();

//	while (tm.getMilliseconds() < 1000) {
//		std::stringstream ss; ss << id;
//		bf.contains(ss.str());
//		i++;
//		tm.stop();
//	}
//
//	std::cout << "serialized: " << i << std::endl;
//
//	i = 0;
//	tm.start();
//
//	while (tm.getMilliseconds() < 1000) {
//		bf.contains(id.toString());
//		i++;
//		tm.stop();
//	}
//
//	std::cout << "toString: " << i << std::endl;
//
//	i = 0;
//	tm.start();

	while (tm.getMilliseconds() < 1000) {
		id.isIn(bf);
		i++;
		tm.stop();
	}

	std::cout << std::endl << i << " hashes per second" << std::endl;
}

void TestBundleID::payloadLengthTest(void)
{
	dtn::data::Bundle b;
	b.source = dtn::data::EID("dtn://node1/app1");
	b.destination = dtn::data::EID("dtn://node2/app2");
	b.lifetime = 3600;
	b.timestamp = 12345678;
	b.sequencenumber = 1234;

	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
	{
		ibrcommon::BLOB::iostream stream = ref.iostream();
		for (int i = 0; i < 10; ++i)
			(*stream) << "hello world" << std::flush;
	}

	b.push_back(ref);

	dtn::data::BundleID id(b);

	CPPUNIT_ASSERT_EQUAL((dtn::data::Length)110, id.getPayloadLength());
}

void TestBundleID::copyTest(void)
{
	dtn::data::Bundle b;
	b.set(dtn::data::PrimaryBlock::FRAGMENT, true);
	b.source = dtn::data::EID("dtn://node1/app1");
	b.destination = dtn::data::EID("dtn://node2/app2");
	b.lifetime = 3600;
	b.timestamp = 12345678;
	b.sequencenumber = 1234;
	b.fragmentoffset = 12;
	b.appdatalength = 200;

	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
	{
		ibrcommon::BLOB::iostream stream = ref.iostream();
		for (int i = 0; i < 10; ++i)
			(*stream) << "hello world" << std::flush;
	}

	b.push_back(ref);

	dtn::data::BundleID id1(b);
	dtn::data::BundleID id2(id1);
	dtn::data::BundleID id3;
	id3 = id1;

	CPPUNIT_ASSERT_EQUAL((const dtn::data::BundleID&)b, id1);
	CPPUNIT_ASSERT_EQUAL((const dtn::data::BundleID&)b, id2);
	CPPUNIT_ASSERT_EQUAL((const dtn::data::BundleID&)b, id3);

	dtn::data::MetaBundle meta = dtn::data::MetaBundle::create(id3);
	CPPUNIT_ASSERT_EQUAL((const dtn::data::BundleID&)b, (const dtn::data::BundleID&)meta);
}
