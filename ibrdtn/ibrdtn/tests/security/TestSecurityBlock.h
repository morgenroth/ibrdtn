/*
 * TestSecurityBlock.h
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
#include <ibrdtn/security/SecurityKey.h>
#include <string>

#ifndef TESTSECURITYBLOCK_H_
#define TESTSECURITYBLOCK_H_

class SecurityStringKey : public dtn::security::SecurityKey
{
public:
	SecurityStringKey() : _data() {};
	virtual ~SecurityStringKey() {};

	void setData(const std::string &data)
	{
		_data = data;
	}

	virtual const std::string getData() const
	{
		return _data;
	}

private:
	std::string _data;
};

class TestSecurityBlock : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestSecurityBlock);
	CPPUNIT_TEST (localBABTest);
	CPPUNIT_TEST (serializeBABTest);
	CPPUNIT_TEST (permutationBabPibTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void localBABTest(void);
	void serializeBABTest(void);
	void permutationBabPibTest(void);

private:
	dtn::security::SecurityKey pubkey;
	dtn::security::SecurityKey pkey;
	SecurityStringKey _babkey;

	std::string _testdata;
};

#endif /* TESTSECURITYBLOCK_H_ */
