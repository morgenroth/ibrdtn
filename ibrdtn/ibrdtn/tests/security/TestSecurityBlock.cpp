/*
 * TestSecurityBlock.cpp
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

#include "security/TestSecurityBlock.h"
#include <cppunit/extensions/HelperMacros.h>

#include <ibrcommon/data/File.h>
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrdtn/security/BundleAuthenticationBlock.h>
#include <ibrdtn/security/PayloadIntegrityBlock.h>
#include <ibrdtn/security/SecurityKey.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/Serializer.h>
#include <ibrcommon/data/BLOB.h>
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION (TestSecurityBlock);

void TestSecurityBlock::setUp(void)
{
	_testdata = "Hallo Welt!";

	_babkey.reference = dtn::data::EID("dtn://source");
	_babkey.setData("0123456789");

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

void TestSecurityBlock::tearDown(void)
{
}

void TestSecurityBlock::localBABTest(void)
{
	dtn::data::Bundle b;
	b.source = dtn::data::EID("dtn://source/app");
	b.destination = dtn::data::EID("dtn://destination/app");
	b.procflags |= dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON;
	b.lifetime = 3600;

	const dtn::data::PayloadBlock &p = b.push_back<dtn::data::PayloadBlock>();
	ibrcommon::BLOB::Reference ref = p.getBLOB();

	// write some data
	{
		ibrcommon::BLOB::iostream io = ref.iostream();
		(*io) << "Hallo Welt";
	}

	// sign the bundle
	dtn::security::BundleAuthenticationBlock::auth(b, _babkey);

	// sign the bundle
	dtn::security::BundleAuthenticationBlock::verify(b, _babkey);
}

void TestSecurityBlock::serializeBABTest(void)
{
	// create a temporary buffer to the serialization tests
	std::stringstream ss;
	dtn::data::DefaultSerializer ds(ss);

	// we create an serialize a new bundle with SecurityBlock appended
	{
		dtn::data::Bundle b;
		b.source = dtn::data::EID("dtn://source/app");
		b.destination = dtn::data::EID("dtn://destination/app");
		b.procflags |= dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON;
		b.lifetime = 3600;

		const dtn::data::PayloadBlock &p = b.push_back<dtn::data::PayloadBlock>();
		ibrcommon::BLOB::Reference ref = p.getBLOB();

		// write some data
		{
			ibrcommon::BLOB::iostream io = ref.iostream();
			(*io) << "Hallo Welt";
		}

		// sign the bundle
		dtn::security::BundleAuthenticationBlock::auth(b, _babkey);

		ds << b;
	}

	dtn::data::DefaultDeserializer dd(ss);

	// deserialize the bundle
	{
		dtn::data::Bundle b;
		dd >> b;

		// verify the bundle
		dtn::security::BundleAuthenticationBlock::verify(b, _babkey);
	}
}

void TestSecurityBlock::permutationBabPibTest(void)
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

	// sign the bundle
	dtn::security::BundleAuthenticationBlock::auth(b, _babkey);

	// check the number of block
	CPPUNIT_ASSERT_EQUAL((size_t)4, b.size());

	// verify the bundle
	dtn::security::BundleAuthenticationBlock::verify(b, _babkey);
	dtn::security::BundleAuthenticationBlock::strip(b);

	// check the number of block
	CPPUNIT_ASSERT_EQUAL((size_t)2, b.size());

	for (dtn::data::Bundle::const_iterator i = b.begin(); i != b.end(); ++i) {
		CPPUNIT_ASSERT_THROW(dynamic_cast<const dtn::security::BundleAuthenticationBlock&>(**i), std::bad_cast);
	}

	dtn::security::PayloadIntegrityBlock::verify(b, pubkey);
	dtn::security::PayloadIntegrityBlock::strip(b);

	CPPUNIT_ASSERT_EQUAL((size_t)1, b.size());
}
