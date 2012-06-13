/*
 * TestCompressedPayloadBlock.h
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
#include <ibrdtn/data/CompressedPayloadBlock.h>

#ifndef TESTCOMPRESSEDPAYLOADBLOCK_H_
#define TESTCOMPRESSEDPAYLOADBLOCK_H_

class TestCompressedPayloadBlock : public CPPUNIT_NS :: TestFixture
{
	CPPUNIT_TEST_SUITE (TestCompressedPayloadBlock);
	CPPUNIT_TEST (compressTest);
	CPPUNIT_TEST (extractTest);
	CPPUNIT_TEST_SUITE_END ();

public:
	void setUp (void);
	void tearDown (void);

protected:
	void compressTest(void);
	void extractTest(void);
};

#endif /* TESTCOMPRESSEDPAYLOADBLOCK_H_ */
