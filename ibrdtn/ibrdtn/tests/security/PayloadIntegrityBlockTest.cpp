/*
 * PayloadIntegrityBlockTest.cpp
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

#include "security/PayloadIntegrityBlockTest.h"
#include <ibrdtn/security/PayloadIntegrityBlock.h>
#include <ibrdtn/security/SecurityKey.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrcommon/data/File.h>

#include <cppunit/extensions/HelperMacros.h>

CPPUNIT_TEST_SUITE_REGISTRATION (PayloadIntegrityBlockTest);

void PayloadIntegrityBlockTest::setUp(void)
{
	_testdata = "Hallo Welt!";

	pubkey.type = dtn::security::SecurityKey::KEY_PUBLIC;
	pubkey.file = ibrcommon::File("test-key.pem");
	pubkey.reference = dtn::data::EID("dtn://test");

	pkey.type = dtn::security::SecurityKey::KEY_PRIVATE;
	pkey.file = ibrcommon::File("test-key.pem");
	pkey.reference = pubkey.reference;

	if (!pubkey.file.exists())
	{
		throw ibrcommon::Exception("test-key.pem file not exists!");
	}
}

void PayloadIntegrityBlockTest::tearDown(void)
{
}

void PayloadIntegrityBlockTest::signTest(void)
{
	dtn::data::Bundle b;
	b.source = dtn::data::EID("dtn://test");
	b.destination = pkey.reference;

	// add payload block
	dtn::data::PayloadBlock &p = b.push_back<dtn::data::PayloadBlock>();

	// write some payload
	(*p.getBLOB().iostream()) << _testdata << std::flush;

	// sign the bundle with PIB
	dtn::security::PayloadIntegrityBlock::sign(b, pkey, pkey.reference);

	// check the number of block, should be two
	CPPUNIT_ASSERT_EQUAL((size_t)2, b.size());
}

void PayloadIntegrityBlockTest::verifyTest(void)
{
	dtn::data::Bundle b;
	b.source = dtn::data::EID("dtn://test");
	b.destination = pubkey.reference;

	// add payload block
	dtn::data::PayloadBlock &p = b.push_back<dtn::data::PayloadBlock>();

	// write some payload
	(*p.getBLOB().iostream()) << _testdata << std::flush;

	// sign the bundle with PIB
	dtn::security::PayloadIntegrityBlock::sign(b, pkey, pkey.reference);

	std::stringstream ss;
	dtn::data::DefaultSerializer dser(ss);
	dser << b;

	dtn::security::PayloadIntegrityBlock::verify(b, pubkey);

	// decode the bundle
	ss.clear();
	dtn::data::DefaultDeserializer ddser(ss);
	dtn::data::Bundle recv_b;
	ddser >> recv_b;

	dtn::security::PayloadIntegrityBlock::verify(recv_b, pubkey);
}

void PayloadIntegrityBlockTest::verifySkipTest(void)
{
	dtn::data::Bundle b;
	b.source = dtn::data::EID("dtn://test");
	b.destination = pubkey.reference;

	// add payload block
	dtn::data::PayloadBlock &p = b.push_back<dtn::data::PayloadBlock>();

	// write some payload
	(*p.getBLOB().iostream()) << _testdata << std::flush;

	// sign the bundle with PIB
	dtn::security::PayloadIntegrityBlock::sign(b, pkey, pkey.reference);

	b.set(dtn::data::PrimaryBlock::FRAGMENT, true);
	b.fragmentoffset = 12;
	b.appdatalength = p.getLength();

	CPPUNIT_ASSERT_THROW(dtn::security::PayloadIntegrityBlock::verify(b, pubkey), dtn::security::VerificationSkippedException);

	CPPUNIT_ASSERT_EQUAL(false, b.get(dtn::data::PrimaryBlock::DTNSEC_STATUS_VERIFIED));
}

void PayloadIntegrityBlockTest::verifyCompromisedTest(void)
{
	dtn::data::Bundle b;
	b.source = dtn::data::EID("dtn://test");
	b.destination = pubkey.reference;

	// add payload block
	dtn::data::PayloadBlock &p = b.push_back<dtn::data::PayloadBlock>();

	// write some payload
	(*p.getBLOB().iostream()) << _testdata << std::flush;

	// sign the bundle with PIB
	dtn::security::PayloadIntegrityBlock::sign(b, pkey, pkey.reference);

	std::stringstream ss;
	dtn::data::DefaultSerializer dser(ss);
	dser << b;

	dtn::security::PayloadIntegrityBlock::verify(b, pubkey);

	// decode the bundle
	ss.clear();
	dtn::data::DefaultDeserializer ddser(ss);
	dtn::data::Bundle recv_b;
	ddser >> recv_b;

	// change the payload
	dtn::data::PayloadBlock &payload = recv_b.find<dtn::data::PayloadBlock>();
	ibrcommon::BLOB::Reference ref = payload.getBLOB();
	{
		ibrcommon::BLOB::iostream stream = ref.iostream();
		(*stream) << "This is a compromised payload." << std::flush;
	}

	CPPUNIT_ASSERT_THROW(dtn::security::PayloadIntegrityBlock::verify(recv_b, pubkey), dtn::security::VerificationFailedException);
}
