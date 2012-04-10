/*
 * TestBundleList.cpp
 *
 *  Created on: 02.06.2010
 *      Author: morgenro
 */

#include "data/TestBundleList.h"
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/utils/Clock.h>
#include <iostream>
#include <cstdlib>

CPPUNIT_TEST_SUITE_REGISTRATION (TestBundleList);

void TestBundleList::setUp()
{
	dtn::utils::Clock::quality = 1;
	list = new TestBundleList::DerivedBundleList();
}

void TestBundleList::tearDown()
{
	delete list;
}

TestBundleList::DerivedBundleList::DerivedBundleList()
 : counter(0)
{

}

TestBundleList::DerivedBundleList::~DerivedBundleList()
{
}

void TestBundleList::DerivedBundleList::eventBundleExpired(const ExpiringBundle&)
{
	//std::cout << "Bundle expired " << b.bundle.toString() << std::endl;
	counter++;
}

void TestBundleList::genbundles(DerivedBundleList &l, int number, int offset, int max)
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

void TestBundleList::containTest(void)
{
	DerivedBundleList &l = (*list);

	dtn::data::Bundle b1;
	b1._source = dtn::data::EID("dtn:test");
	b1._timestamp = 1;
	b1._sequencenumber = 1;

	dtn::data::Bundle b2;
	b2._source = dtn::data::EID("dtn:test");
	b2._timestamp = 2;
	b2._sequencenumber = 3;

	CPPUNIT_ASSERT(l.contains(b1) == false);
	CPPUNIT_ASSERT(l.contains(b2) == false);

	l.add(b1);

	CPPUNIT_ASSERT(l.contains(b1) == true);
	CPPUNIT_ASSERT(l.contains(b2) == false);

	l.add(b2);

	CPPUNIT_ASSERT(l.contains(b1) == true);
	CPPUNIT_ASSERT(l.contains(b2) == true);

	l.remove(b1);

	CPPUNIT_ASSERT(l.contains(b1) == false);
	CPPUNIT_ASSERT(l.contains(b2) == true);

	l.add(b1);

	CPPUNIT_ASSERT(l.contains(b1) == true);
	CPPUNIT_ASSERT(l.contains(b2) == true);

	l.remove(b2);

	CPPUNIT_ASSERT(l.contains(b1) == true);
	CPPUNIT_ASSERT(l.contains(b2) == false);
}

void TestBundleList::orderTest(void)
{
	DerivedBundleList &l = (*list);

	CPPUNIT_ASSERT(l.counter == 0);

	genbundles(l, 1000, 0, 500);
	genbundles(l, 1000, 600, 1000);

	for (int i = 0; i < 550; i++)
	{
		l.expire(i);
	}

	CPPUNIT_ASSERT(l.counter == 1000);

	for (int i = 0; i < 1050; i++)
	{
		l.expire(i);
	}

	CPPUNIT_ASSERT(l.counter == 2000);
}

