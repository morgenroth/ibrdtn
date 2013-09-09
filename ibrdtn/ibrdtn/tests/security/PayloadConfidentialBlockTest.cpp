/*
 * PayloadConfidentialBlockTest.cpp
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

#include "security/PayloadConfidentialBlockTest.h"
#include <ibrdtn/security/PayloadConfidentialBlock.h>
#include <ibrdtn/security/SecurityKey.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrcommon/data/File.h>

#include <cppunit/extensions/HelperMacros.h>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION (PayloadConfidentialBlockTest);


void PayloadConfidentialBlockTest::setUp(void)
{
	_testdata = "Hallo Welt!";
}

void PayloadConfidentialBlockTest::tearDown(void)
{
}

void PayloadConfidentialBlockTest::encryptTest(void)
{
	dtn::security::SecurityKey pubkey;
	pubkey.type = dtn::security::SecurityKey::KEY_PUBLIC;
	pubkey.file = ibrcommon::File("test-key.pem");
	pubkey.reference = dtn::data::EID("dtn://test");

	dtn::security::SecurityKey pkey;
	pkey.type = dtn::security::SecurityKey::KEY_PRIVATE;
	pkey.file = ibrcommon::File("test-key.pem");

	if (!pubkey.file.exists())
	{
		throw ibrcommon::Exception("test-key.pem file not exists!");
	}

	dtn::data::Bundle b;
	b.source = dtn::data::EID("dtn://test");
	b.destination = pubkey.reference;

	// add payload block
	dtn::data::PayloadBlock &p = b.push_back<dtn::data::PayloadBlock>();

	// write some payload
	(*p.getBLOB().iostream()) << _testdata << std::flush;

	// verify the payload
	{
		ibrcommon::BLOB::iostream stream = p.getBLOB().iostream();
		std::stringstream ss; ss << (*stream).rdbuf();

		if (ss.str().length() != _testdata.length())
		{
			throw ibrcommon::Exception("wrong payload size, write failed!");
		}

		if (ss.str() != _testdata)
		{
			throw ibrcommon::Exception("payload write failed!");
		}
	}

	// encrypt the payload block
	dtn::security::PayloadConfidentialBlock::encrypt(b, pubkey, b.source);

	// verify the encryption
	{
		ibrcommon::BLOB::iostream stream = p.getBLOB().iostream();
		std::stringstream ss; ss << (*stream).rdbuf();

		if (ss.str().length() != _testdata.length())
		{
			throw ibrcommon::Exception("wrong payload size, encryption failed!");
		}

		if (ss.str() == _testdata)
		{
			throw ibrcommon::Exception("encryption failed!");
		}
	}

	// check the number of block, should be two
	CPPUNIT_ASSERT_EQUAL((size_t)2, b.size());
}

std::string PayloadConfidentialBlockTest::getHex(std::istream &stream)
{
	stream.clear();
	stream.seekg(0);

	std::stringstream buf;
	unsigned int c = stream.get();

	while (!stream.eof())
	{
		buf << std::hex << c << ":";
		c = stream.get();
	}

	buf << std::flush;

	return buf.str();
}

void PayloadConfidentialBlockTest::decryptTest(void)
{
	dtn::security::SecurityKey pubkey;
	pubkey.type = dtn::security::SecurityKey::KEY_PUBLIC;
	pubkey.file = ibrcommon::File("test-key.pem");
	pubkey.reference = dtn::data::EID("dtn://source");

	if (!pubkey.file.exists())
	{
		throw ibrcommon::Exception("test-key.pem file not exists!");
	}

	dtn::data::Bundle b;
	b.source = pubkey.reference;
	b.source.setApplication("test");

	b.destination = dtn::data::EID("dtn://destination/test");

	encrypt(pubkey, b);

	// check the number of block, should be two
	CPPUNIT_ASSERT_EQUAL((size_t)2, b.size());

	std::stringstream ss;
	dtn::data::DefaultSerializer dser(ss);
	dser << b;

	// decrypt unserialized bundle
	decrypt(pubkey, b);

	// check the number of block, should be one
	CPPUNIT_ASSERT_EQUAL((size_t)1, b.size());

	/**
	 * decrypt the bundle (payload only)
	 */
	ss.clear();
	dtn::data::DefaultDeserializer ddser(ss);
	dtn::data::Bundle recv_b;
	ddser >> recv_b;

	// check the number of block, should be two
	CPPUNIT_ASSERT_EQUAL((size_t)2, recv_b.size());

	decrypt(pubkey, recv_b);

	// check the number of block, should be one
	CPPUNIT_ASSERT_EQUAL((size_t)1, recv_b.size());
}

void PayloadConfidentialBlockTest::encrypt(const dtn::security::SecurityKey &pubkey, dtn::data::Bundle &b)
{
	/**
	 * Add a payload with some payload data to the bundle and
	 * encrypt it.
	 */
	// add payload block
	dtn::data::PayloadBlock &p = b.push_back<dtn::data::PayloadBlock>();

	// write some payload
	(*p.getBLOB().iostream()) << _testdata << std::flush;

	// encrypt the payload block
	dtn::security::PayloadConfidentialBlock::encrypt(b, pubkey, b.source);
}

void PayloadConfidentialBlockTest::decrypt(const dtn::security::SecurityKey &pubkey, dtn::data::Bundle &b)
{
	dtn::security::SecurityKey pkey;
	pkey.type = dtn::security::SecurityKey::KEY_PRIVATE;
	pkey.file = ibrcommon::File("test-key.pem");
	pkey.reference = pubkey.reference;

	dtn::security::PayloadConfidentialBlock::decrypt(b, pkey);

	// verify the payload
	dtn::data::PayloadBlock &p = b.find<dtn::data::PayloadBlock>();

	ibrcommon::BLOB::iostream stream = p.getBLOB().iostream();
	std::stringstream ss; ss << (*stream).rdbuf();

	if (ss.str().length() != _testdata.length())
	{
		throw ibrcommon::Exception("wrong payload size, decryption failed!");
	}

	if (ss.str() != _testdata)
	{
		throw ibrcommon::Exception("decryption failed!");
	}
}
