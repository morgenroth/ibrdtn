/*
 * TestExtensionBlock.cpp
 *
 *  Created on: 16.01.2013
 *      Author: morgenro
 */

#include "data/TestExtensionBlock.h"
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/Serializer.h>
#include <ibrcommon/data/BLOB.h>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION (TestExtensionBlock);

void TestExtensionBlock::setUp()
{
}

void TestExtensionBlock::tearDown()
{
}

void TestExtensionBlock::deserializeUnknownBlock(void)
{
	dtn::data::Bundle b;
	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();
	(*ref.iostream()) << "hello world";

	dtn::data::PayloadBlock &payload = b.push_back(ref);

	std::stringstream ss;
	dtn::data::DefaultSerializer ds(ss);

	// serialize the primary block
	ds << (const dtn::data::PrimaryBlock&)b;

	// fake unknown block
	unsigned char type = 140;
	ss.put(type);					// block type
	ss << dtn::data::SDNV(0);	// block flags
	ss << dtn::data::SDNV(5);	// block size
	ss << "12345";

	// serialize payload block
	ds << (const dtn::data::Block&)payload;

	dtn::data::DefaultDeserializer dds(ss);
	dtn::data::Bundle dest;

	// deserialize the bundle
	dds >> dest;

	// check unknown block
	const dtn::data::ExtensionBlock &unknown = dynamic_cast<const dtn::data::ExtensionBlock&>(*dest.begin());
	CPPUNIT_ASSERT_EQUAL((unsigned char)140, unknown.getType());
	ibrcommon::BLOB::Reference uref = unknown.getBLOB();

	{
		ibrcommon::BLOB::iostream stream = uref.iostream();
		CPPUNIT_ASSERT_EQUAL( (size_t)5, stream.size() );
	}

	// check payload block
	dtn::data::PayloadBlock &dest_payload = dest.find<dtn::data::PayloadBlock>();

	CPPUNIT_ASSERT_EQUAL((size_t)11, dest_payload.getBLOB().iostream().size());
}

