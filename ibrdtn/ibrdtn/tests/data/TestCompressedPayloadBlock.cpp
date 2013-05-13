/*
 * TestCompressedPayloadBlock.cpp
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

#include "data/TestCompressedPayloadBlock.h"
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrcommon/data/BLOB.h>

CPPUNIT_TEST_SUITE_REGISTRATION (TestCompressedPayloadBlock);

void TestCompressedPayloadBlock::setUp()
{
}

void TestCompressedPayloadBlock::tearDown()
{
}

void TestCompressedPayloadBlock::compressTest(void)
{
	dtn::data::Bundle b;
	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();

	// generate some test data
	{
		ibrcommon::BLOB::iostream stream = ref.iostream();
		for (int i = 0; i < 10000; ++i)
		{
			(*stream) << "0123456789";
		}
	}

	// add a payload block
	const dtn::data::Length origin_psize = b.push_back(ref).getLength();

	dtn::data::CompressedPayloadBlock::compress(b, dtn::data::CompressedPayloadBlock::COMPRESSION_ZLIB);

	dtn::data::PayloadBlock &p = b.find<dtn::data::PayloadBlock>();

	CPPUNIT_ASSERT(origin_psize > p.getLength());
}

void TestCompressedPayloadBlock::extractTest(void)
{
	dtn::data::Bundle b;
	ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();

	// generate some test data
	{
		ibrcommon::BLOB::iostream stream = ref.iostream();
		for (int i = 0; i < 10000; ++i)
		{
			(*stream) << "0123456789";
		}
	}

	// add a payload block
	const dtn::data::Length origin_psize = b.push_back(ref).getLength();

	dtn::data::CompressedPayloadBlock::compress(b, dtn::data::CompressedPayloadBlock::COMPRESSION_ZLIB);
	dtn::data::CompressedPayloadBlock::extract(b);

	dtn::data::PayloadBlock &p = b.find<dtn::data::PayloadBlock>();

	CPPUNIT_ASSERT_EQUAL(origin_psize, p.getLength());

	// detailed check of the payload
	{
		ibrcommon::BLOB::iostream stream = p.getBLOB().iostream();
		for (int i = 0; i < 10000; ++i)
		{
			char buf[10];
			(*stream).read(buf, 10);
			CPPUNIT_ASSERT_EQUAL((*stream).gcount(), (std::streamsize)10);
			CPPUNIT_ASSERT_EQUAL(std::string(buf, 10), std::string("0123456789"));
		}
	}
}
