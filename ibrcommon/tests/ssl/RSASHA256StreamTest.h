/*
 * RSASHA256StreamTest.h
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

#ifndef RSASHA256STREAMTEST_H_
#define RSASHA256STREAMTEST_H_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <string>

class RSASHA256StreamTest : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (RSASHA256StreamTest);
	CPPUNIT_TEST (rsastream_test01);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void rsastream_test01();

private:
	std::string getHex(std::istream &stream);
};

#endif /* RSASHA256STREAMTEST_H_ */
