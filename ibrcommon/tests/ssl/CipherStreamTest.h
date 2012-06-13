/*
 * CipherStreamTest.h
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

#ifndef CIPHERSTREAMTEST_H_
#define CIPHERSTREAMTEST_H_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ibrcommon/ssl/CipherStream.h>

#include <istream>
#include <string>
#include <sstream>

class CipherStreamTest : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (CipherStreamTest);
	CPPUNIT_TEST (xorstream_test01);
	CPPUNIT_TEST (xorstream_test02);
	CPPUNIT_TEST (xorstream_test03);
	CPPUNIT_TEST (xorstream_test04);

	CPPUNIT_TEST (aesstream_test01);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void xorstream_test01();
	void xorstream_test02();
	void xorstream_test03();
	void xorstream_test04();

	void aesstream_test01();

private:
	// stores some plain data generated while setUp
	char _plain_data[1024];

	std::string getHex(std::istream &stream);
	std::string getHex(char (&data)[1024]);

	// compares a data array with the plain data
	bool compare(char (&encrypted)[1024]);
};

#endif /* CIPHERSTREAMTEST_H_ */
