/*
 * SQLiteBundleStorageTestSuite.h
 *
 *  Created on: 08.04.2010
 *      Author: Myrtus
 */

#ifndef SQLITEBUNDLESTORAGETESTSUITE_H_
#define SQLITEBUNDLESTORAGETESTSUITE_H_

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "src/core/SQLiteBundleStorage.h"

namespace dtn
{
namespace testsuite
{
	class SQLiteBundleStorageTestSuite : public CPPUNIT_NS::TestFixture{

		CPPUNIT_TEST_SUITE(SQLiteBundleStorageTestSuite);
		CPPUNIT_TEST(Konstruktortest);
		CPPUNIT_TEST(insertTest);
		CPPUNIT_TEST(getTest);
		CPPUNIT_TEST(storeNodeRoutingTest);
		CPPUNIT_TEST(getNodeRoutingTest);
		CPPUNIT_TEST(storeRoutingTest);
		CPPUNIT_TEST(getRoutingTest);
		CPPUNIT_TEST(getBundleRoutingInfoTest);
		CPPUNIT_TEST(storeBundleRoutingInfoTest);
		CPPUNIT_TEST(deleteDepricatedTest);
		CPPUNIT_TEST(getByEIDTest);
		CPPUNIT_TEST(unblockTest);
		CPPUNIT_TEST(fragmentationTest);
		CPPUNIT_TEST(consistenceTest);
		CPPUNIT_TEST(clearALLTest);
		CPPUNIT_TEST(getBlockTest);
		CPPUNIT_TEST(removeTest);
		CPPUNIT_TEST(storeBundleTimeTest);
		CPPUNIT_TEST(insertTimeTest);
		CPPUNIT_TEST(readTimeTest);
		CPPUNIT_TEST_SUITE_END();

	public:
		SQLiteBundleStorageTestSuite();
		virtual ~SQLiteBundleStorageTestSuite();

        void setUp ();
        void tearDown ();
        static void signal(string name);

	protected:
        void Konstruktortest();
        void insertTest();
        void getTest();
        void storeNodeRoutingTest();
        void getNodeRoutingTest();
        void storeRoutingTest();
        void getRoutingTest();
        void getBundleRoutingInfoTest();
        void storeBundleRoutingInfoTest();
        void deleteDepricatedTest();
        void fragmentationTest();
        void getByEIDTest();
        void consistenceTest();
        void clearALLTest();
        void removeBundleRoutingTest();
        void removeNodeRoutingTest();
        void removeRoutingTest();
        void removeTest();
        void emptyTest();
        void setPriorityTest();
        void getBundlesBySourceTest();
        void getBundlesByDestination();
        void getBlockTest();
        void occupiedSpaceTest();
        void getBundleBySizeTest();
        void storeBundleTimeTest();
        void insertTimeTest();
        void readTimeTest();

	private:
        sqlite3_stmt* prepareStatement(string sqlquery, sqlite3 *database);
        void getBundle(dtn::data::Bundle &bundle);
        void getBundle(dtn::data::Bundle &bundle, string source, string dest, int size = 2000);
        void getBundle(dtn::data::Bundle &bundle, string source, string dest, ibrcommon::File &file);
        void dbInsert();
        void storeBlocks (const data::Bundle &bundle);
        void clearFolder(string pfad);
        list<data::Bundle> getFragment(data::Bundle &bundle, int second, int third, int total);
        list<data::BundleID>  insertTime(int number, int size, vector<vector<double> > &result, string payloadfile);
        void deleteTime(list<data::BundleID> &idList,vector<vector<double> > &result);
        static list<std::string> finishedThreads;

        std::string _path;
	};
}
}

#endif /* SQLITEBUNDLESTORAGETESTSUITE_H_ */
