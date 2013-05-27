/*
 * PayloadConfidentialBlockTest.h
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

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/security/SecurityKey.h>
#include <iostream>
#include <string>

#ifndef PAYLOADCONFIDENTIALBLOCKTEST_H_
#define PAYLOADCONFIDENTIALBLOCKTEST_H_

class PayloadConfidentialBlockTest : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (PayloadConfidentialBlockTest);
	CPPUNIT_TEST (encryptTest);
	CPPUNIT_TEST (decryptTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void encryptTest(void);
	void decryptTest(void);

private:
	void encrypt(const dtn::security::SecurityKey &pubkey, dtn::data::Bundle &b);
	void decrypt(const dtn::security::SecurityKey &pubkey, dtn::data::Bundle &b);
	std::string getHex(std::istream &stream);
	std::string _testdata;
};

#endif /* PAYLOADCONFIDENTIALBLOCKTEST_H_ */
