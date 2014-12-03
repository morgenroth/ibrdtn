/*
 * TestSDNV.h
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

#ifndef TESTSDNV_H_
#define TESTSDNV_H_

class TestSDNV : public CPPUNIT_NS :: TestFixture {
	CPPUNIT_TEST_SUITE (TestSDNV);
	CPPUNIT_TEST (testLength);
	CPPUNIT_TEST (testSerialize);
	CPPUNIT_TEST (testSizeT);
	CPPUNIT_TEST (testMax);
	CPPUNIT_TEST (testMax32);
	CPPUNIT_TEST (testOutOfRange);
	CPPUNIT_TEST (testBitset);
	CPPUNIT_TEST (testTrim);
	CPPUNIT_TEST_SUITE_END ();

	static void hexdump(char c);

public:
	void setUp (void);
	void tearDown (void);

	void testSerialize(void);
	void testLength(void);
	void testSizeT(void);
	void testOutOfRange(void);
	void testMax(void);
	void testMax32(void);
	void testBitset(void);
	void testTrim(void);
};

#endif /* TESTSDNV_H_ */
