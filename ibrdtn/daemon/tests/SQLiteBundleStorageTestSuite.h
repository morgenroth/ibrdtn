/*
 * SQLiteBundleStorageTestSuite.h
 *
 *   Copyright 2011 Matthias Myrtus
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
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
