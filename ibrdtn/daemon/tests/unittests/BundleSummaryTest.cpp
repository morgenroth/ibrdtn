/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BundleSummaryTest.cpp
/// @brief       CPPUnit-Tests for class BundleSummary
/// @author      Author Name (email@mail.address)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

/*
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

#include "BundleSummaryTest.hh"
#include <ibrdtn/utils/Clock.h>
#include "src/routing/BundleSummary.h"
#include <sstream>

CPPUNIT_TEST_SUITE_REGISTRATION(BundleSummaryTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'BundleSummary' ===*/
void BundleSummaryTest::testAdd()
{
	/* test signature (const dtn::data::MetaBundle bundle) */
	dtn::routing::BundleSummary l;

	// define bundle one
	dtn::data::Bundle b1;
	b1._lifetime = 20;
	b1._timestamp = 0;
	b1._sequencenumber = 23;
	b1._source = dtn::data::EID("dtn://test1/app0");

	CPPUNIT_ASSERT(!l.contains(b1));

	l.add(b1);

	CPPUNIT_ASSERT(l.contains(b1));
}

void BundleSummaryTest::testRemove()
{
	/* test signature (const dtn::data::MetaBundle bundle) */
	dtn::routing::BundleSummary l;

	// define bundle one
	dtn::data::Bundle b1;
	b1._lifetime = 20;
	b1._timestamp = 0;
	b1._sequencenumber = 23;
	b1._source = dtn::data::EID("dtn://test1/app0");

	l.add(b1);

	CPPUNIT_ASSERT(l.contains(b1));

	l.remove(b1);

	CPPUNIT_ASSERT(!l.contains(b1));
}

void BundleSummaryTest::testClear()
{
	/* test signature () */
	dtn::routing::BundleSummary l;

	// define bundle one
	dtn::data::Bundle b1;
	b1._lifetime = 20;
	b1._timestamp = 0;
	b1._sequencenumber = 23;
	b1._source = dtn::data::EID("dtn://test1/app0");

	l.add(b1);

	CPPUNIT_ASSERT(l.contains(b1));

	l.clear();

	CPPUNIT_ASSERT(!l.contains(b1));
}

void BundleSummaryTest::testContains()
{
	/* test signature (const dtn::data::BundleID &bundle) const */
	dtn::routing::BundleSummary l;

	// define bundle one
	dtn::data::Bundle b1;
	b1._lifetime = 20;
	b1._timestamp = 0;
	b1._sequencenumber = 23;
	b1._source = dtn::data::EID("dtn://test1/app0");

	// define bundle two
	dtn::data::Bundle b2;
	b2._lifetime = 20;
	b2._timestamp = 0;
	b2._sequencenumber = 23;
	b2._source = dtn::data::EID("dtn://test2/app3");

	// generate some bundles
	genbundles(l, 1000, 0, 500);

	// the bundle one should not be in the bundle list
	CPPUNIT_ASSERT(!l.contains(b1));

	// the bundle two should not be in the bundle list
	CPPUNIT_ASSERT(!l.contains(b2));

	// add bundle one to the bundle list
	l.add(b1);

	// the bundle one should be in the bundle list
	CPPUNIT_ASSERT(l.contains(b1));

	// generate more bundles
	genbundles(l, 1000, 0, 500);

	// the bundle one should be in the bundle list
	CPPUNIT_ASSERT(l.contains(b1));

	// the bundle two should not be in the bundle list
	CPPUNIT_ASSERT(!l.contains(b2));

	// add bundle two to the bundle list
	l.add(b2);

	// the bundle two should be in the bundle list
	CPPUNIT_ASSERT(l.contains(b2));

	// remove bundle one
	l.remove(b2);

	// the bundle two should not be in the bundle list
	CPPUNIT_ASSERT(!l.contains(b2));

	// the bundle one should be in the bundle list
	CPPUNIT_ASSERT(l.contains(b1));
}

void BundleSummaryTest::testGetSummaryVector()
{
	/* test signature () const */
	dtn::routing::BundleSummary l;

	// define bundle one
	dtn::data::Bundle b1;
	b1._lifetime = 20;
	b1._timestamp = 0;
	b1._sequencenumber = 23;
	b1._source = dtn::data::EID("dtn://test1/app0");

	l.add(b1);

	CPPUNIT_ASSERT(l.getSummaryVector().contains(b1));
}

/*=== END   tests for class 'BundleSummary' ===*/

void BundleSummaryTest::setUp()
{
	dtn::utils::Clock::rating = 1;
}

void BundleSummaryTest::tearDown()
{
}

void BundleSummaryTest::expireTest(void)
{
	dtn::routing::BundleSummary l;

	CPPUNIT_ASSERT(l.size() == 0);

	genbundles(l, 1000, 0, 500);
	genbundles(l, 1000, 600, 1000);

	for (int i = 0; i < 550; i++)
	{
		l.expire(i);
	}

	CPPUNIT_ASSERT(l.size() == 1000);

	for (int i = 0; i < 1050; i++)
	{
		l.expire(i);
	}

	CPPUNIT_ASSERT(l.size() == 0);
}

void BundleSummaryTest::genbundles(dtn::routing::BundleSummary &l, int number, int offset, int max)
{
	int range = max - offset;
	dtn::data::Bundle b;

	for (int i = 0; i < number; i++)
	{
		int random_integer = offset + (rand() % range);

		b._lifetime = random_integer;
		b._timestamp = 0;
		b._sequencenumber = random_integer;

		stringstream ss; ss << rand();

		b._source = dtn::data::EID("dtn://node" + ss.str() + "/application");

		l.add(b);
	}
}

std::string BundleSummaryTest::getHex(std::istream &stream)
{
	std::stringstream buf;
	unsigned int c = stream.get();

	while (!stream.eof())
	{
		buf << std::hex << c << ":";
		c = stream.get();
	}

	buf << std::flush;

	return buf.str();
}
