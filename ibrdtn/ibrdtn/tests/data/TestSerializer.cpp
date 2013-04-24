/*
 * TestSerializer.cpp
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

#include "data/TestSerializer.h"
#include <ibrdtn/data/EID.h>
#include <cppunit/extensions/HelperMacros.h>

#include <ibrcommon/thread/MutexLock.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/Serializer.h>
#include <ibrdtn/data/BundleFragment.h>
#include <ibrdtn/data/AgeBlock.h>
#include <iostream>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION (TestSerializer);

void TestSerializer::setUp(void)
{
}

void TestSerializer::tearDown(void)
{
}

void TestSerializer::serializer_separate01(void)
{
	dtn::data::Bundle b;
	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();

	stringstream ss1;
	dtn::data::PayloadBlock &p1 = b.push_back(ref);
	p1.addEID(dtn::data::EID("dtn://test1234/app1234"));

	stringstream ss2;
	dtn::data::PayloadBlock &p2 = b.push_back(ref);
	p2.addEID(dtn::data::EID("dtn://test1234/app1234"));

	dtn::data::SeparateSerializer(ss1) << p1;
	dtn::data::SeparateSerializer(ss2) << p2;

	ss1.clear(); ss1.seekp(0); ss1.seekg(0);
	ss2.clear(); ss2.seekp(0); ss2.seekg(0);

	dtn::data::Bundle b2;

	dtn::data::SeparateDeserializer(ss1, b2).readBlock();
	dtn::data::SeparateDeserializer(ss2, b2).readBlock();

	CPPUNIT_ASSERT_EQUAL( b2.size(), b.size() );
}

void TestSerializer::serializer_cbhe01(void)
{
	dtn::data::Bundle b;

	dtn::data::EID src("ipn:1.2");
	dtn::data::EID dst("ipn:2.3");
	dtn::data::EID report("ipn:6.1");

	std::pair<size_t, size_t> cbhe_eid = src.getCompressed();

	b._source = src;
	b.destination = dst;
	b.reportto = report;

	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
	b.push_back(ref);

	std::stringstream ss;
	dtn::data::DefaultSerializer(ss) << b;

	dtn::data::Bundle b2;
	ss.clear(); ss.seekp(0); ss.seekg(0);
	dtn::data::DefaultDeserializer(ss) >> b2;

	CPPUNIT_ASSERT( b._source == b2._source );
	CPPUNIT_ASSERT( b.destination == b2.destination );
	CPPUNIT_ASSERT( b.reportto == b2.reportto );
	CPPUNIT_ASSERT( b.custodian == b2.custodian );
}

void TestSerializer::serializer_cbhe02(void)
{
	dtn::data::Bundle b;

	dtn::data::EID src("ipn:1.2");
	dtn::data::EID dst("ipn:2.3");

	b._source = src;
	b.destination = dst;

	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();

	{
		ibrcommon::BLOB::iostream stream = ref.iostream();
		(*stream) << "0123456789";
	}

	b.push_back(ref);

	std::stringstream ss;
	dtn::data::DefaultSerializer(ss) << b;

	CPPUNIT_ASSERT_EQUAL((size_t)33, ss.str().length());
}

void TestSerializer::serializer_primaryblock_length(void)
{
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://node1/app1");
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
	b.push_front<dtn::data::AgeBlock>();

	std::stringstream ss;
	dtn::data::DefaultSerializer ds(ss);
	ds << (dtn::data::PrimaryBlock&)b;

	size_t written_len = ss.tellp();
	size_t calc_len = ds.getLength((dtn::data::PrimaryBlock&)b);

	CPPUNIT_ASSERT_EQUAL(written_len, calc_len);
}

void TestSerializer::serializer_block_length(void)
{
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://node1/app1");
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
	b.push_front<dtn::data::AgeBlock>();

	for (dtn::data::Bundle::iterator iter = b.begin(); iter != b.end(); ++iter)
	{
		const dtn::data::Block &block = (**iter);

		std::stringstream ss;
		dtn::data::DefaultSerializer ds(ss);
		ds << block;

		size_t written_len = ss.tellp();
		size_t calc_len = ds.getLength(block);

		CPPUNIT_ASSERT_EQUAL(written_len, calc_len);
	}
}

void TestSerializer::serializer_bundle_length(void)
{
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://node1/app1");
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
	b.push_front<dtn::data::AgeBlock>();

	std::stringstream ss;
	dtn::data::DefaultSerializer ds(ss);
	ds << b;

	size_t written_len = ss.tellp();
	size_t calc_len = ds.getLength(b);

	CPPUNIT_ASSERT_EQUAL(written_len, calc_len);
}

void TestSerializer::serializer_fragment_one(void)
{
	dtn::data::Bundle b;
	b._source = dtn::data::EID("dtn://node1/app1");
	b.destination = dtn::data::EID("dtn://node2/app2");
	b.lifetime = 3600;
	b.timestamp = 12345678;
	b.sequencenumber = 1234;

	std::stringstream ss;
	dtn::data::DefaultSerializer ds(ss);

	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();

	// generate some payload data
	{
		ibrcommon::BLOB::iostream ios = ref.iostream();
		for (int i = 0; i < 100; ++i)
		{
			(*ios) << "0123456789";
		}
	}

	// add a payload block to the bundle
	b.push_back(ref);

	// serialize the bundle in fragments with only 100 bytes of payload
	for (int i = 0; i < 10; ++i)
	{
		ds << dtn::data::BundleFragment(b, i * 100, 100);
	}

	// jump to the first byte in the stringstream
	ss.seekg(0);

	for (int i = 0; i < 10; ++i)
	{
		dtn::data::Bundle fb;
		dtn::data::DefaultDeserializer(ss) >> fb;
		CPPUNIT_ASSERT(fb.get(dtn::data::PrimaryBlock::FRAGMENT));
		CPPUNIT_ASSERT_EQUAL(fb.find<dtn::data::PayloadBlock>().getLength(), (size_t)100);
	}
}

void TestSerializer::serializer_ipn_compression_length(void)
{
	dtn::data::Bundle ipnbundle;
	ipnbundle._source = dtn::data::EID("ipn:5.2");
	ipnbundle.destination = dtn::data::EID("ipn:1.2");

	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
	{
		ibrcommon::BLOB::iostream stream = ref.iostream();
		for (int i = 0; i < 10; ++i)
			(*stream) << "hello world" << std::flush;
	}

	ipnbundle.push_back(ref);

	ipnbundle.lifetime = 3600;

	ipnbundle.push_front<dtn::data::AgeBlock>();

	std::stringstream ss;
	dtn::data::DefaultSerializer ds(ss);
	ds << ipnbundle;

	size_t written_len = ss.tellp();
	size_t calc_len = ds.getLength(ipnbundle);

	CPPUNIT_ASSERT_EQUAL(written_len, calc_len);
}
