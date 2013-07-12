/*
 * TestEID.h
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

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#ifndef TESTEID_H_
#define TESTEID_H_

class TestEID : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestEID);
	CPPUNIT_TEST (testCBHEwithApplication);
	CPPUNIT_TEST (testCBHEwithoutApplication);
	CPPUNIT_TEST (testCBHEConstructorNumbers);
	CPPUNIT_TEST (testCBHEConstructorSchemeSsp);
	CPPUNIT_TEST (testCBHEEquals);
	CPPUNIT_TEST (testCBHEHost);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void testCBHEwithApplication(void);
	void testCBHEwithoutApplication(void);
	void testCBHEConstructorNumbers(void);
	void testCBHEConstructorSchemeSsp(void);
	void testCBHEEquals(void);
	void testCBHEHost(void);

};

#endif /* TESTEID_H_ */
