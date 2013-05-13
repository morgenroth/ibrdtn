/*
 * TestPlainSerializer.cpp
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

#include "api/TestPlainSerializer.h"
#include <ibrdtn/data/EID.h>
#include <cppunit/extensions/HelperMacros.h>

#include <ibrcommon/thread/MutexLock.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/AgeBlock.h>
#include <ibrdtn/data/ScopeControlHopLimitBlock.h>
#include <ibrdtn/api/PlainSerializer.h>
#include <ibrdtn/data/Serializer.h>
#include <ibrdtn/data/BundleFragment.h>
#include <ibrdtn/data/BundleBuilder.h>
#include <iostream>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION (TestPlainSerializer);

void TestPlainSerializer::hexdump(char c) {
  std::cout << std::hex << (int)c << " ";
}

void TestPlainSerializer::setUp(void)
{
}

void TestPlainSerializer::tearDown(void)
{
}

void TestPlainSerializer::plain_serializer_inversion(void)
{
	std::stringstream ss, ss1, ss2, payloadss;
	dtn::api::PlainSerializer serializer(ss);
	dtn::api::PlainDeserializer deserializer(ss);
	dtn::data::Bundle b1, b2;

	/* create Bundle */
	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
	payloadss << "test payload" << std::endl;
	(*ref.iostream()) << payloadss.rdbuf();
	dtn::data::PayloadBlock &p1 = b1.push_back(ref);

	/* set all possible flags */
	p1.set( dtn::data::Block::REPLICATE_IN_EVERY_FRAGMENT, true );
	p1.set( dtn::data::Block::TRANSMIT_STATUSREPORT_IF_NOT_PROCESSED, true );
	p1.set( dtn::data::Block::DELETE_BUNDLE_IF_NOT_PROCESSED, true );
	p1.set( dtn::data::Block::DISCARD_IF_NOT_PROCESSED, true );
	p1.set( dtn::data::Block::BLOCK_CONTAINS_EIDS, true );

	/* add an EID to the payload block */
	p1.addEID(dtn::data::EID("dtn://test1234/app1234"));

	/* add scope control hop block */
	b1.push_back<dtn::data::ScopeControlHopLimitBlock>();

	/* add unknown extension block */
	dtn::data::BundleBuilder builder(b1);
	dtn::data::Block &ext_block = builder.insert(42, 0);

	dtn::data::ExtensionBlock &ext_block_cast = dynamic_cast<dtn::data::ExtensionBlock&>(ext_block);
	ibrcommon::BLOB::Reference ext_ref = ext_block_cast.getBLOB();
	{
		ibrcommon::BLOB::iostream ext_stream = ext_ref.iostream();
		(*ext_stream) << "Hello World" << std::flush;
	}

	ext_block_cast.set( dtn::data::Block::FORWARDED_WITHOUT_PROCESSED, true );

	/* serialize b1 and deserialize it into b2 */
	serializer << b1;
	deserializer >> b2;

	/* serialize b1 and b2 into stringstreams */
	dtn::data::DefaultSerializer(ss1) << b1;
	dtn::data::DefaultSerializer(ss2) << b2;

//	std::string ss1_data = ss1.str();
//	std::string ss2_data = ss2.str();
//
//	// Debugging
//	std::cout << "\n---\n" << ss.str() << "\n---" << std::endl;
//
//	std::cout << "\n---\n" << std::endl;
//	std::for_each(ss1_data.begin(), ss1_data.end(), hexdump);
//	std::cout << "\n---\n" << std::endl;
//
//	std::cout << "\n---\n" << std::endl;
//	std::for_each(ss2_data.begin(), ss2_data.end(), hexdump);
//	std::cout << "\n---\n" << std::endl;

	/* compare strings */
	CPPUNIT_ASSERT_EQUAL( 0, ss1.str().compare(ss2.str()) );
}
