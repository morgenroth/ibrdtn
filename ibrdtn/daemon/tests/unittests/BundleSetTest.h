/*
 * BundleSetTest.h
 *
 *  Created on: May 14, 2013
 *      Author: goltzsch
 */

#ifndef BUNDLESETTEST_H_
#define BUNDLESETTEST_H_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "config.h"
#include "storage/BundleStorage.h"
#include "data/BundleSet.h"
#include <vector>

class BundleSetTest : public CppUnit::TestFixture {
	private:

		ibrtest::EventSwitchLoop *esl;

		void containTest();
		void orderTest();
		class ExpiredBundleCounter : public dtn::data::BundleSet::Listener
		{
		public:
			ExpiredBundleCounter();
			virtual ~ExpiredBundleCounter();

			void eventBundleExpired(const dtn::data::MetaBundle &b) throw ();
			int counter;
		};
		void genbundles(dtn::data::BundleSet &l, int number, int offset, int max);

	public:
#define CPPUNIT_TEST_ALL_STORAGES(testMethod) \
		testCounter = 0; \
		for (size_t i = 0; i < _set_names.size(); i++) { \
		  CPPUNIT_TEST_SUITE_ADD_TEST( \
		    ( new CPPUNIT_NS::TestCaller<TestFixtureType>( \
		    	context.getTestNameFor( std::string(#testMethod) + " (" + _set_names[i] + ")" ), \
		    	&TestFixtureType::testMethod, \
		        context.makeFixture() ) ) ); \
		}

		void containTest();
		void orderTest();

		void setUp();
		void tearDown();

		CPPUNIT_TEST_SUITE(BundleSetTest);

		_set_names.push_back("MemoryBundleSet");

#ifdef HAVE_SQLITE
		_set_names.push_back("SQLiteBundleSet");
#endif

		CPPUNIT_TEST_ALL_STORAGES(containTest);
		CPPUNIT_TEST_ALL_STORAGES(orderTest);
		CPPUNIT_TEST_SUITE_END();

		static size_t testCounter;
		static dtn::storage::BundleStorage *_storage;
		static std::vector<std::string> _set_names;
};
#endif /* BUNDLESETTEST_H_ */
