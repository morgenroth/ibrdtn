/* $Id: templateengine.py 2241 2006-05-22 07:58:58Z fischer $ */

///
/// @file        BundleStorageTest.hh
/// @brief       CPPUnit-Tests for bundle storages
/// @author      Johannes Morgenroth (morgenroth@ibr.cs.tu-bs.de)
/// @date        Created at 2010-11-01
/// 
/// @version     $Revision: 2241 $
/// @note        Last modification: $Date: 2006-05-22 09:58:58 +0200 (Mon, 22 May 2006) $
///              by $Author: fischer $
///

 
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "config.h"
#include "storage/BundleStorage.h"
#include "Component.h"
#include "../tools/EventSwitchLoop.h"
#include <vector>

#ifndef BUNDLESTORAGETEST_HH
#define BUNDLESTORAGETEST_HH
class BundleStorageTest : public CppUnit::TestFixture {
	private:
		ibrtest::EventSwitchLoop *esl;

		void testStore(dtn::storage::BundleStorage &storage);
		void testRemove(dtn::storage::BundleStorage &storage);
		void testAgeBlock(dtn::storage::BundleStorage &storage);
		void testClear(dtn::storage::BundleStorage &storage);
		void testEmpty(dtn::storage::BundleStorage &storage);
		void testCount(dtn::storage::BundleStorage &storage);
		void testSize(dtn::storage::BundleStorage &storage);
		void testSizeExpiration(dtn::storage::BundleStorage &storage);
		void testReleaseCustody(dtn::storage::BundleStorage &storage);
		void testRaiseEvent(dtn::storage::BundleStorage &storage);
		void testConcurrentStoreGet(dtn::storage::BundleStorage &storage);
		void testRestore(dtn::storage::BundleStorage &storage);
		void testExpiration(dtn::storage::BundleStorage &storage);
		void testDistinctDestinations(dtn::storage::BundleStorage &storage);
		void testSelector(dtn::storage::BundleStorage &storage);
		void testRemoveBloomfilter(dtn::storage::BundleStorage &storage);
		void testDoubleStore(dtn::storage::BundleStorage &storage);
		void testGet(dtn::storage::BundleStorage &storage);
		void testFaultyGet(dtn::storage::BundleStorage &storage);
		void testFaultyStore(dtn::storage::BundleStorage &storage);
		void testQueryBloomFilter(dtn::storage::BundleStorage &storage);
		void testFragment(dtn::storage::BundleStorage &storage);
		void testContains(dtn::storage::BundleStorage &storage);
		void testInfo(dtn::storage::BundleStorage &storage);

	public:
#define CPPUNIT_TEST_ALL_STORAGES(testMethod) \
		testCounter = 0; \
		for (size_t i = 0; i < _storage_names.size(); i++) { \
		  CPPUNIT_TEST_SUITE_ADD_TEST( \
		    ( new CPPUNIT_NS::TestCaller<TestFixtureType>( \
		    	context.getTestNameFor( std::string(#testMethod) + " (" + _storage_names[i] + ")" ), \
		    	&TestFixtureType::testMethod, \
		        context.makeFixture() ) ) ); \
		}

		void testStore();
		void testRemove();
		void testAgeBlock();
		void testClear();
		void testEmpty();
		void testCount();
		void testSize();
		void testSizeExpiration();
		void testReleaseCustody();
		void testRaiseEvent();
		void testConcurrentStoreGet();
		void testRestore();
		void testExpiration();
		void testDistinctDestinations();
		void testSelector();
		void testDoubleStore();
		void testGet();
		void testFaultyGet();
		void testFaultyStore();
		void testQueryBloomFilter();
		void testFragment();
		void testContains();
		void testInfo();

		void setUp();
		void tearDown();

		CPPUNIT_TEST_SUITE(BundleStorageTest);

		_storage_names.push_back("MemoryBundleStorage");
		_storage_names.push_back("SimpleBundleStorage");

#ifdef HAVE_SQLITE
		_storage_names.push_back("SQLiteBundleStorage");
#endif

		CPPUNIT_TEST_ALL_STORAGES(testStore);
		CPPUNIT_TEST_ALL_STORAGES(testRemove);
		CPPUNIT_TEST_ALL_STORAGES(testAgeBlock);
		CPPUNIT_TEST_ALL_STORAGES(testClear);
		CPPUNIT_TEST_ALL_STORAGES(testEmpty);
		CPPUNIT_TEST_ALL_STORAGES(testCount);
		CPPUNIT_TEST_ALL_STORAGES(testSize);
		CPPUNIT_TEST_ALL_STORAGES(testSizeExpiration);
		CPPUNIT_TEST_ALL_STORAGES(testReleaseCustody);
		CPPUNIT_TEST_ALL_STORAGES(testRaiseEvent);
		CPPUNIT_TEST_ALL_STORAGES(testConcurrentStoreGet);
		CPPUNIT_TEST_ALL_STORAGES(testRestore);
		CPPUNIT_TEST_ALL_STORAGES(testExpiration);
		CPPUNIT_TEST_ALL_STORAGES(testDistinctDestinations);
		CPPUNIT_TEST_ALL_STORAGES(testSelector);
		CPPUNIT_TEST_ALL_STORAGES(testDoubleStore);
		CPPUNIT_TEST_ALL_STORAGES(testGet);
		CPPUNIT_TEST_ALL_STORAGES(testFaultyGet);
		CPPUNIT_TEST_ALL_STORAGES(testFaultyStore);
		CPPUNIT_TEST_ALL_STORAGES(testQueryBloomFilter);
		CPPUNIT_TEST_ALL_STORAGES(testFragment);
		CPPUNIT_TEST_ALL_STORAGES(testContains);
		CPPUNIT_TEST_ALL_STORAGES(testInfo);
		CPPUNIT_TEST_SUITE_END();

		static size_t testCounter;
		static dtn::storage::BundleStorage *_storage;
		static std::vector<std::string> _storage_names;
};
#endif /* BUNDLESTORAGETEST_HH */
