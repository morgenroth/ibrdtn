/**

 * SQLiteBundleSetTest.cpp
 * test for SQLiteBundleSet
 * created 22.04.13
 * goltzsch

 */

#include "SQLiteBundleSetTest.hh"

#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>
#include <cstdlib>


CPPUNIT_TEST_SUITE_REGISTRATION(SQLiteBundleSetTest);

/*========================== tests below ==========================*/

/*=== BEGIN tests for class 'SQLiteBundleSet' ===*/
void SQLiteBundleSetTest::containTest(void){
	ExpiredBundleCounter ebc;

	/*
	dtn::data::BundleSet l =_sqliteStorage->createBundleSet(&ebc,1024);
	//TODO -> factory

	dtn::data::Bundle b1;
	b1.source = dtn::data::EID("dtn:test");
	b1.timestamp = 1;
	b1.sequencenumber = 1;

	dtn::data::Bundle b2;
	b2.source = dtn::data::EID("dtn:test");
	b2.timestamp = 2;
	b2.sequencenumber = 3;

	CPPUNIT_ASSERT(l.has(b1) == false);
	CPPUNIT_ASSERT(l.has(b2) == false);

	l.add(b1);

	CPPUNIT_ASSERT(l.has(b1) == true);
	CPPUNIT_ASSERT(l.has(b2) == false);

	l.add(b2);

	CPPUNIT_ASSERT(l.has(b1) == true);
	CPPUNIT_ASSERT(l.has(b2) == true);

	l.clear();

	CPPUNIT_ASSERT(l.has(b1) == false);
	CPPUNIT_ASSERT(l.has(b2) == false);

	l.add(b1);

	CPPUNIT_ASSERT(l.has(b1) == true);
	CPPUNIT_ASSERT(l.has(b2) == false);

	l.add(b2);

	CPPUNIT_ASSERT(l.has(b1) == true);
	CPPUNIT_ASSERT(l.has(b2) == true);
	*/
}
void SQLiteBundleSetTest::orderTest(void){
	/*
	dtn::data::BundleSet l =_sqliteStorage->createBundleSet();

	CPPUNIT_ASSERT(l.size() == 0);

	genbundles(l, 10, 0, 500);
	genbundles(l, 10, 600, 1000);

	CPPUNIT_ASSERT(l.size() == 20);


	for (int i = 0; i < 550; ++i)
	{
		l.expire(i);
	}

	CPPUNIT_ASSERT(l.size() == 10);

	for (int i = 0; i < 1050; ++i)
	{
		l.expire(i);
	}

	CPPUNIT_ASSERT(l.size() == 0);*/
}


/*=== END   tests for class 'SQLiteBundleStorage' ===*/

void SQLiteBundleSetTest::genbundles(dtn::data::BundleSet &l, int number, int offset, int max)
{
	int range = max - offset;
	dtn::data::MetaBundle b;

	for (int i = 0; i < number; ++i)
	{
		int random_integer = offset + (rand() % range);

		b.lifetime = random_integer;
		b.expiretime = random_integer;

		b.timestamp = 1;
		b.sequencenumber = random_integer;

		stringstream ss; ss << rand();

		b.source = dtn::data::EID("dtn://node" + ss.str() + "/application");


		l.add(b);
	}
}
SQLiteBundleSetTest::ExpiredBundleCounter::ExpiredBundleCounter()
 : counter(0)
{

}

SQLiteBundleSetTest::ExpiredBundleCounter::~ExpiredBundleCounter()
{
}

void SQLiteBundleSetTest::ExpiredBundleCounter::eventBundleExpired(const dtn::data::MetaBundle&) throw ()
{
	counter++;
}
void SQLiteBundleSetTest::setUp()
{
	ibrcommon::File workdir("/tmp/sqlite-bundle-set-test");
	//TODO nach merge enablen
	//if (workdir.exists())
		//workdir.remove(true);
	ibrcommon::File::createDirectory(workdir);

	_sqliteStorage = new dtn::storage::SQLiteBundleStorage(workdir,10000);
	_sqliteStorage->initialize();
}

void SQLiteBundleSetTest::tearDown()
{
	delete _sqliteStorage;
}

