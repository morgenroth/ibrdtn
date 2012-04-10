/*
 * TestDictionary.cpp
 *
 *  Created on: 23.06.2010
 *      Author: morgenro
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

	std::pair<size_t, size_t> ref1 = dict.getRef( dtn::data::EID("dtn://node1") );
	std::pair<size_t, size_t> ref2 = dict.getRef( dtn::data::EID("dtn://node1/applikation") );

	CPPUNIT_ASSERT(ref1.first == ref2.first);

	CPPUNIT_ASSERT(ref1.second == 4);
}
