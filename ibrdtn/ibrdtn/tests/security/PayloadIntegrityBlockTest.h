/*
 * PayloadIntegrityBlockTest.h
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

#include <ibrdtn/security/SecurityKey.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef PAYLOADINTEGRITYBLOCKTEST_H_
#define PAYLOADINTEGRITYBLOCKTEST_H_

class PayloadIntegrityBlockTest : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (PayloadIntegrityBlockTest);
	CPPUNIT_TEST (signTest);
	CPPUNIT_TEST (verifyTest);
	CPPUNIT_TEST (verifySkipTest);
	CPPUNIT_TEST (verifyCompromisedTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void signTest(void);
	void verifyTest(void);
	void verifySkipTest(void);
	void verifyCompromisedTest(void);

private:
	dtn::security::SecurityKey pubkey;
	dtn::security::SecurityKey pkey;

	std::string _testdata;
};

#endif /* PAYLOADINTEGRITYBLOCKTEST_H_ */
