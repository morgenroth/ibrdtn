/**
 * SQLiteBundleSetTest.hh
 * test for SQLiteBundleSet
 * created 22.04.13
 * goltzsch
 */
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
//#include <ibrdtn/data/BundleSet.h>
#include "storage/SQLiteBundleSet.h"

#include "storage/SQLiteBundleStorage.h"

#ifndef SQLITEBUNDLESETTEST_HH
#define SQLITEBUNDLESETTEST_HH
class SQLiteBundleSetTest : public CppUnit::TestFixture {
		CPPUNIT_TEST_SUITE(SQLiteBundleSetTest);
			CPPUNIT_TEST(containTest);
			CPPUNIT_TEST(orderTest);
		CPPUNIT_TEST_SUITE_END();
	public:

		void setUp();
		void tearDown();
	protected:
		/*=== BEGIN tests for class 'SQLiteBundleStorage' ===*/
		void orderTest(void);
		void containTest(void);
		/*=== END   tests for class 'SQLiteBundleStorage' ===*/
	private:

		dtn::storage::SQLiteBundleStorage* _sqliteStorage;

		class ExpiredBundleCounter : public dtn::data::BundleSet::Listener
		{
		public:
			ExpiredBundleCounter();
			virtual ~ExpiredBundleCounter();

			void eventBundleExpired(const dtn::data::MetaBundle &b) throw ();
			int counter;
		};


		void genbundles(dtn::data::BundleSet &l, int number, int offset, int max);


};
#endif /* SQLITEBUNDLESETTEST_HH */
