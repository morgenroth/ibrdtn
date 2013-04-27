/*
 * TestDictionary.cpp
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

#include "data/TestDictionary.h"
#include <ibrdtn/data/EID.h>
#include <cppunit/extensions/HelperMacros.h>


CPPUNIT_TEST_SUITE_REGISTRATION (TestDictionary);

void TestDictionary::setUp(void)
{
}

void TestDictionary::tearDown(void)
{
}

void TestDictionary::mainTest(void)
{
	dtn::data::Dictionary dict;

	dict.add( dtn::data::EID("dtn://node1") );

	dict.add( dtn::data::EID("dtn://node1/applikation") );

	dict.add( dtn::data::EID("dtn://node2") );

	dtn::data::Dictionary::Reference ref1 = dict.getRef( dtn::data::EID("dtn://node1") );
	dtn::data::Dictionary::Reference ref2 = dict.getRef( dtn::data::EID("dtn://node1/applikation") );

	CPPUNIT_ASSERT(ref1.first == ref2.first);

	CPPUNIT_ASSERT(ref1.second == 4);
}
