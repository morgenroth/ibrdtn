/*
 * RSASHA256StreamTest.cpp
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

#include "ssl/RSASHA256StreamTest.h"

#include <ibrcommon/ssl/RSASHA256Stream.h>
#include <iostream>
#include <sstream>

#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

CPPUNIT_TEST_SUITE_REGISTRATION (RSASHA256StreamTest);

void RSASHA256StreamTest::setUp()
{
}

void RSASHA256StreamTest::tearDown()
{
}

std::string RSASHA256StreamTest::getHex(std::istream &stream)
{
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

void RSASHA256StreamTest::rsastream_test01()
{
	EVP_PKEY* pkey = EVP_PKEY_new();
	FILE * pkey_file = fopen("test-key.pem", "r");
	pkey = PEM_read_PrivateKey(pkey_file, &pkey, NULL, NULL);
	fclose(pkey_file);

	EVP_PKEY* pubkey = EVP_PKEY_new();
	FILE * pubkey_file = fopen("test-key.pem", "r");
	pubkey = PEM_read_PUBKEY(pubkey_file, &pubkey, NULL, NULL);
	fclose(pubkey_file);

	ibrcommon::RSASHA256Stream rsa_stream(pkey);
	rsa_stream << "Hello World" << std::flush;

	std::pair< const int, const std::string > sign = rsa_stream.getSign();

	ibrcommon::RSASHA256Stream rsa_stream_verify(pubkey, true);
	rsa_stream_verify << "Hello World" << std::flush;

	int verify = rsa_stream_verify.getVerification(sign.second);

	ibrcommon::RSASHA256Stream rsa_stream_verify_fail(pubkey, true);
	rsa_stream_verify_fail << "Hello World failure" << std::flush;

	int verify_fail = rsa_stream_verify_fail.getVerification(sign.second);

	EVP_PKEY_free(pkey);
	EVP_PKEY_free(pubkey);

	CPPUNIT_ASSERT_EQUAL(1, verify);
	CPPUNIT_ASSERT_EQUAL(0, verify_fail);
}
