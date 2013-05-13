/*
 * TestUtils.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Stephen Roettger <roettger@ibr.cs.tu-bs.de>
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

#ifndef TESTUTILS_H_
#define TESTUTILS_H_

class TestUtils : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestUtils);
	CPPUNIT_TEST (tokenizeEmptyTest);
	CPPUNIT_TEST (tokenizeSizeTest);
	CPPUNIT_TEST (tokenizeSizeSpacesTest);
	CPPUNIT_TEST (tokenizeLastItemTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void tokenizeEmptyTest(void);
	void tokenizeSizeTest(void);
	void tokenizeSizeSpacesTest(void);
	void tokenizeLastItemTest(void);
};


#endif /* TESTUTILS_H_ */

