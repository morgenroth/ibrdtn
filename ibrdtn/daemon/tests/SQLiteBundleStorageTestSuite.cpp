/*
 * SQLiteBundleStorageTestSuite.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 * Written-by: Matthias Myrtus
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

#include "tests/SQLiteBundleStorageTestSuite.h"
#include <ibrdtn/data/BundleID.h>
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrdtn/data/EID.h>
#include "src/core/TimeEvent.h"
#include <ibrcommon/TimeMeasurement.h>
#include <sqlite3.h>

namespace dtn{
namespace testsuite{

	CPPUNIT_TEST_SUITE_REGISTRATION (SQLiteBundleStorageTestSuite);

	sqlite3_stmt* SQLiteBundleStorageTestSuite::prepareStatement(string sqlquery, sqlite3 *database){
		// prepare the statement
		sqlite3_stmt *statement;
		int err = sqlite3_prepare_v2(database, sqlquery.c_str(), sqlquery.length(), &statement, 0);
		if ( err != SQLITE_OK )
		{
			std::cerr << "SQLiteBundlestorage: failure in prepareStatement: " << err << " with Query: " << sqlquery << std::endl;
		}
		return statement;
	}

	void SQLiteBundleStorageTestSuite::dbInsert(){
		int file;
		fstream filestream;
		sqlite3 *database;
		sqlite3_stmt *ins;
		int err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		ins = prepareStatement("INSERT INTO Bundles (BundleID, Source, Destination, Reportto, Custodian, ProcFlags, Timestamp, Sequencenumber, Lifetime, Fragmentoffset, Appdatalength, TTL, Size) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);", database);
		for(int i = 0; i < 100; i++){
			stringstream text,filename;
			text << i;
			sqlite3_bind_text(ins,1,text.str().c_str(),text.str().length(),SQLITE_TRANSIENT);
			sqlite3_bind_text(ins,2,text.str().c_str(),text.str().length(),SQLITE_TRANSIENT);
			sqlite3_bind_text(ins,3,text.str().c_str(),text.str().length(),SQLITE_TRANSIENT);
			sqlite3_bind_text(ins,4,text.str().c_str(),text.str().length(),SQLITE_TRANSIENT);
			sqlite3_bind_text(ins,5,text.str().c_str(),text.str().length(),SQLITE_TRANSIENT);
			sqlite3_bind_int(ins,6,23);
			sqlite3_bind_int(ins,7,42);
			sqlite3_bind_int(ins,8,42);
			sqlite3_bind_int(ins,9,42);
			sqlite3_bind_int(ins,10,42);
			sqlite3_bind_int(ins,11,42);
			sqlite3_bind_int(ins,12,42);
			sqlite3_bind_int(ins,13,42);
			sqlite3_step(ins);
			sqlite3_reset(ins);
			file = sqlite3_last_insert_rowid(database);
			filename << "/home/pennywise/workspace/EmmaSVN/Unittest/Bundles/" << file;
			filestream.open(filename.str().c_str(), ios::out);
			filestream << "hallo";
			filestream.close();
		}
		sqlite3_finalize(ins);
		sqlite3_close(database);
	}

	void SQLiteBundleStorageTestSuite::getBundle(dtn::data::Bundle &bundle){
		getBundle(bundle, "dtn:alice/app3", "dtn:bob/app2");
	}

	void SQLiteBundleStorageTestSuite::getBundle(dtn::data::Bundle &bundle, string source, string dest, int size){
		fstream lala;
		lala.open("/home/pennywise/workspace/EmmaSVN/Unittest/hallo",ios::out|ios::binary);
		for (int i = 1; i<=size; i++){
			if(i<=(size/2))
				lala<< "1";
			else
				lala<<"2";
		}
		lala.close();

		ibrcommon::File file = ibrcommon::File("/home/pennywise/workspace/EmmaSVN/Unittest/hallo");
		if(!file.exists()){
			cerr << "Datei existiert nicht"<<endl;
		}
		ibrcommon::BLOB::Reference payloadblob = ibrcommon::FileBLOB::create(file);
		dtn::data::PayloadBlock *pb = new dtn::data::PayloadBlock(payloadblob);

		bundle.addBlock(pb);
		bundle._source = dtn::data::EID(source);
		bundle._destination = dtn::data::EID(dest);
		bundle._procflags += dtn::data::Bundle::PRIORITY_BIT1;
		bundle._procflags += dtn::data::Bundle::CUSTODY_REQUESTED;
		bundle._reportto = dtn::data::EID(source);

	}

	void SQLiteBundleStorageTestSuite::getBundle(dtn::data::Bundle &bundle, string source, string dest, ibrcommon::File &file){
		if(!file.exists()){
			cerr << "Datei existiert nicht"<<endl;
		}
		ibrcommon::BLOB::Reference payloadblob = ibrcommon::FileBLOB::create(file);
		dtn::data::PayloadBlock *pb = new dtn::data::PayloadBlock(payloadblob);

		bundle.addBlock(pb);
		bundle._source = dtn::data::EID(source);
		bundle._destination = dtn::data::EID(dest);
		bundle._procflags += dtn::data::Bundle::PRIORITY_BIT1;
		bundle._procflags += dtn::data::Bundle::CUSTODY_REQUESTED;
	}

	list<data::Bundle> SQLiteBundleStorageTestSuite::getFragment(data::Bundle &bundle, int second, int third, int total){
		fstream lala,lala2,lala3;
		list<data::Bundle> result;
		lala.open("/home/pennywise/workspace/EmmaSVN/Unittest/hallo",ios::out|ios::binary);
		lala2.open("/home/pennywise/workspace/EmmaSVN/Unittest/hallo2",ios::out|ios::binary);
		lala3.open("/home/pennywise/workspace/EmmaSVN/Unittest/hallo3",ios::out|ios::binary);
		for (int i = 1; i<=total; i++){
			if(i<second)
				lala<< "1";
			if(i>=second && i < third+4)
				if(i<third)
					lala2<<"2";
				else
					lala2<<"3";
			if(i>= third-4){
				if(i>=third)
					lala3<<"3";
				else
					lala3<<"2";
			}

		}
		lala.close();
		lala2.close();
		lala3.close();

		ibrcommon::File file("/home/pennywise/workspace/EmmaSVN/Unittest/hallo");
		ibrcommon::File file2("/home/pennywise/workspace/EmmaSVN/Unittest/hallo2");
		ibrcommon::File file3("/home/pennywise/workspace/EmmaSVN/Unittest/hallo3");
		if(!file.exists()){
			cerr << "Datei existiert nicht"<<endl;
		}
		ibrcommon::BLOB::Reference payloadblob(ibrcommon::FileBLOB::create(file));
		ibrcommon::BLOB::Reference payloadblob2(ibrcommon::FileBLOB::create(file2));
		ibrcommon::BLOB::Reference payloadblob3(ibrcommon::FileBLOB::create(file3));
		dtn::data::PayloadBlock *pb = new dtn::data::PayloadBlock(payloadblob);
		dtn::data::PayloadBlock *pb2 = new dtn::data::PayloadBlock(payloadblob2);
		dtn::data::PayloadBlock *pb3 = new dtn::data::PayloadBlock(payloadblob3);
	//		pb->setBLOB(payloadblob);

		bundle._source = dtn::data::EID("dtn:alice/app3");
		bundle._destination = dtn::data::EID("dtn:bob/app2");
		bundle._procflags += dtn::data::Bundle::PRIORITY_BIT1;
		bundle._procflags += dtn::data::Bundle::CUSTODY_REQUESTED;
		bundle._procflags += data::Bundle::FRAGMENT;
		bundle.clearBlocks();

		data::Bundle frag = bundle;
		data::Bundle frag2 = bundle;

		bundle.addBlock(pb);
		bundle._fragmentoffset = 0;
		bundle._appdatalength = total;
		frag.addBlock(pb2);
		frag._fragmentoffset = second-1;
		frag._appdatalength = total;
		frag2.addBlock(pb3);
		frag2._fragmentoffset = third-5;
		frag2._appdatalength = total;
		result.push_back(bundle);
		result.push_back(frag);
		result.push_back(frag2);

		return result;
	}

	void SQLiteBundleStorageTestSuite::clearFolder(string pfad){
		stringstream rm,mkdir;
		rm << "rm -r " << pfad;
		mkdir << "mkdir "<<pfad;
		system(rm.str().c_str());
		system(mkdir.str().c_str());
	}

	SQLiteBundleStorageTestSuite::SQLiteBundleStorageTestSuite(): _path("/home/pennywise/workspace/EmmaSVN/Unittest"){
	}

	SQLiteBundleStorageTestSuite::~SQLiteBundleStorageTestSuite(){
	}

	void SQLiteBundleStorageTestSuite::setUp(){
		cout << endl;
		clearFolder(_path);
	}

	void SQLiteBundleStorageTestSuite::tearDown(){

	}

	void SQLiteBundleStorageTestSuite::Konstruktortest(){
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
			DIR *directory;
			bool condition1 = false, condition2 = false, condition3 = false;
			struct dirent *dirpointer;
			if((directory=opendir(_path.c_str())) != NULL){
				while((dirpointer=readdir(directory)) != NULL){
					if (!strcmp(((const char*)(*dirpointer).d_name),_storage._BlockTable.c_str()))
						condition1 = true;
					if (!strcmp(((const char*)(*dirpointer).d_name),_storage._FragmentTable.c_str()))
						condition2 = true;
					if (!strcmp(((const char*)(*dirpointer).d_name),_storage.dbFile.c_str()))
						condition3 = true;
				}
			}
			CPPUNIT_ASSERT(condition1);
			CPPUNIT_ASSERT(condition2);
			CPPUNIT_ASSERT(condition3);
		}
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
		}
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
		}
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
		}
	}

	void SQLiteBundleStorageTestSuite::insertTest(){
		string ID;
		int err,filename,TTLo, expire;
		dtn::data::Bundle bundle,bundle2,bundle3;
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
			getBundle(bundle);
			sleep(1);
			getBundle(bundle2);
			sleep(2);
			getBundle(bundle3,"dtn:hallt","dtn:welt");
			data::BundleID BID(bundle);
			ID = BID.toString();
			TTLo = bundle._timestamp + bundle._lifetime;
//			priorityo = 2 * (bundle._procflags & dtn::data::Bundle::PRIORITY_BIT2) + (bundle._procflags & dtn::data::Bundle::PRIORITY_BIT1);
			_storage.store(bundle2);
			_storage.store(bundle3);
			_storage.store(bundle);
			expire = _storage.nextExpiredTime;
		}

		string bundleID, source, destination, reportto, custodian;
		size_t progFlags, time, datalength, ttl, size, seq, life, offset;
		sqlite3 *database;
		sqlite3_stmt *get;
		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get = prepareStatement("SELECT * FROM Bundles WHERE BundleID = ?;",database);
		sqlite3_bind_text(get,1,ID.c_str(),ID.length(),SQLITE_TRANSIENT);
		err = sqlite3_step(get);
		while (err == SQLITE_ROW){
			bundleID 	= (const char*)sqlite3_column_text(get,1);
			source 		= (const char*)sqlite3_column_text(get,2);
			destination = (const char*)sqlite3_column_text(get,3);
			reportto	= (const char*)sqlite3_column_text(get,4);
			custodian	= (const char*)sqlite3_column_text(get,5);
			progFlags	= sqlite3_column_int(get,6);
			time		= sqlite3_column_int64(get,7);
			seq			= sqlite3_column_int64(get,8);
			life		= sqlite3_column_int64(get,9);
			offset		= sqlite3_column_int64(get,10);
			datalength  = sqlite3_column_int64(get,11);
			ttl			= sqlite3_column_int64(get,12);
			err = sqlite3_step(get);
		}
		if(err != SQLITE_DONE){
			std::cerr << "SQLitePrototyp: storeFragment() failure: "<< err << " " << sqlite3_errmsg(database) << endl;
		}
		sqlite3_finalize(get);
		sqlite3_close(database);
		CPPUNIT_ASSERT(bundleID == ID);
		CPPUNIT_ASSERT(bundle._source.getString() == source);
		CPPUNIT_ASSERT(bundle._destination.getString() == destination);
		CPPUNIT_ASSERT(reportto == bundle._reportto.getString());
		CPPUNIT_ASSERT(custodian == bundle._custodian.getString());
		CPPUNIT_ASSERT(progFlags == bundle._procflags);
		CPPUNIT_ASSERT(time == bundle._timestamp);
		CPPUNIT_ASSERT(bundle._sequencenumber == seq);
		CPPUNIT_ASSERT(bundle._lifetime == life);
		CPPUNIT_ASSERT(bundle._fragmentoffset == offset);
		CPPUNIT_ASSERT(datalength == bundle._appdatalength);
		CPPUNIT_ASSERT(TTLo == ttl);
		CPPUNIT_ASSERT(expire == TTLo);
	}
	void SQLiteBundleStorageTestSuite::getTest(){
		int TTLo;
		dtn::data::Bundle bundle,bundle2;
		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
		getBundle(bundle);
		data::BundleID BID(bundle);
		string ID(BID.toString());
		_storage.store(bundle);
		bundle2 = _storage.get(BID);
		CPPUNIT_ASSERT(bundle == bundle2);
		CPPUNIT_ASSERT(bundle._destination == bundle2._destination);
		CPPUNIT_ASSERT(bundle._reportto == bundle2._reportto);
		CPPUNIT_ASSERT(bundle._custodian == bundle2._custodian);
		CPPUNIT_ASSERT(bundle._lifetime == bundle2._lifetime);
		CPPUNIT_ASSERT(bundle._fragmentoffset == bundle2._fragmentoffset);
		const list<data::Block*> oblocks(bundle.getBlocks());
		const list<data::Block*> nblocks(bundle2.getBlocks());
		list<data::Block*>::const_iterator o_it, n_it;
		o_it = oblocks.begin();
		n_it = nblocks.begin();
		CPPUNIT_ASSERT((*o_it)->_procflags == (*n_it)->_procflags);
	}

//	void SQLiteBundleStorageTestSuite::storeBlocksTest(){
//		{
//			fstream lala, blockfile;
//			lala.open("/home/pennywise/workspace/EmmaSVN/Unittest/blocktest",ios::out|ios::binary);
//			blockfile.open("/home/pennywise/workspace/EmmaSVN/Unittest/block",ios::out|ios::binary);
//			for (int i = 1; i<=2000; i++){
//				if(i<=1000)
//					lala << "1";
//				else
//					lala << "2";
//			}
//			lala.close();
//			ibrcommon::File file("/home/pennywise/workspace/EmmaSVN/Unittest/blocktest");
//			if(!file.exists()){
//				cerr << "Datei existiert nicht"<<endl;
//			}
//			ibrcommon::BLOB::Reference payloadblob(ibrcommon::FileBLOB::create(file));
//			dtn::data::PayloadBlock *pb = new data::PayloadBlock(payloadblob);
//			data::Block *block = new data::Block('P');
//			BlockStreamWriter write(blockfile);
//			int written(write.writeBlock(pb));
//			blockfile.close();
//			blockfile.open("/home/pennywise/workspace/EmmaSVN/Unittest/block",ios::in|ios::binary);
//			streams::BlockStreamReader reader(blockfile,block);
//			int read(reader.readBlock());
//			blockfile.close();
//			block->getBLOB().read(tmp);
//			cout <<"Block: "<<tmp.str() <<endl;
//			CPPUNIT_ASSERT( block->getBLOB().getSize() == pb->getBLOB().getSize());
//			CPPUNIT_ASSERT(block->getType() == pb->getType());
//			CPPUNIT_ASSERT(block->_procflags == pb->_procflags);
//			CPPUNIT_ASSERT(written == read);
//			free(block);
//			free(pb);
//		}
//	}

    void SQLiteBundleStorageTestSuite::storeNodeRoutingTest(){
		int err, key;
    	string nodeInfo("Hallo Welt"), resultID, resultInfo;
		data::EID nodeID("dtn:alice");
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
			_storage.storeNodeRoutingInfo(nodeID,42,nodeInfo);
		}
		sqlite3 *database;
		sqlite3_stmt *get;
		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get = prepareStatement("SELECT * FROM NodeRouitngInfo;",database);
		err = sqlite3_step(get);
		resultID = (const char*)sqlite3_column_text(get,1);
		key = sqlite3_column_int(get,2);
		resultInfo = (const char*)sqlite3_column_text(get,3);
		sqlite3_finalize(get);
		sqlite3_close(database);
		CPPUNIT_ASSERT(resultID == nodeID.getString());
		CPPUNIT_ASSERT(key == 42);
		CPPUNIT_ASSERT(resultInfo == nodeInfo);
    }

    void SQLiteBundleStorageTestSuite::getNodeRoutingTest(){
    	{
    		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
    	}
    	int err;
    	string nodeInfo("Hallo Welt"), resultID, resultInfo;
		data::EID nodeID("dtn:alice");
		sqlite3 *database;
		sqlite3_stmt *set;
		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		set = prepareStatement("INSERT INTO NodeRouitngInfo (EID,Key,Routing) VALUES (?,?,?);",database);
		sqlite3_bind_text(set,1,"dtn:alice",9,SQLITE_TRANSIENT);
		sqlite3_bind_int(set,2,42);
		sqlite3_bind_text(set,3,"Hallo Welt",10,SQLITE_TRANSIENT);
		err = sqlite3_step(set);
		sqlite3_reset(set);

		sqlite3_bind_text(set,1,"dtn:alicee",10,SQLITE_TRANSIENT);
		sqlite3_bind_int(set,2,12);
		sqlite3_bind_text(set,3,"error",5,SQLITE_TRANSIENT);
		err = sqlite3_step(set);
		sqlite3_reset(set);

		sqlite3_bind_text(set,1,"dtn:Alice",9,SQLITE_TRANSIENT);
		sqlite3_bind_int(set,2,13);
		sqlite3_bind_text(set,3,"error",5,SQLITE_TRANSIENT);
		sqlite3_step(set);
		sqlite3_finalize(set);
		sqlite3_close(database);
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
			resultInfo = _storage.getNodeRoutingInfo(nodeID,42);
		}
		cout << "result: " << resultInfo <<endl;
		CPPUNIT_ASSERT(resultInfo == nodeInfo);
    }

    void SQLiteBundleStorageTestSuite::storeRoutingTest(){
    	int err;
    	string nodeInfo("Hallo Welt"), resultID, resultInfo;
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
			_storage.storeRoutingInfo(42 , "Hallo Welt");
			_storage.storeRoutingInfo(42 , "Hallo Welt");
		}
		sqlite3 *database;
		sqlite3_stmt *get;
		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get = prepareStatement("SELECT * FROM Routing",database);
		err = sqlite3_step(get);
		CPPUNIT_ASSERT(err == SQLITE_ROW);
		CPPUNIT_ASSERT(sqlite3_column_int(get,1) == 42);
		string result = (const char*)sqlite3_column_text(get,2);
		CPPUNIT_ASSERT(result == "Hallo Welt");
		sqlite3_close(database);
    }

    void SQLiteBundleStorageTestSuite::getRoutingTest(){
    	{
    		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
    	}
    	int err;
    	sqlite3 *database;
		sqlite3_stmt *set;
		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		set = prepareStatement("INSERT INTO Routing (Key,Routing) VALUES (?,?);",database);
		sqlite3_bind_int(set,1,42);
		sqlite3_bind_text(set,2,"Hallo Welt",10,SQLITE_TRANSIENT);
		sqlite3_step(set);
		sqlite3_reset(set);

		sqlite3_bind_int(set,1,23);
		sqlite3_bind_text(set,2,"error",5,SQLITE_TRANSIENT);
		sqlite3_step(set);
		sqlite3_reset(set);

		sqlite3_bind_int(set,1,2323);
		sqlite3_bind_text(set,2,"error",5,SQLITE_TRANSIENT);
		sqlite3_step(set);
		sqlite3_finalize(set);
		sqlite3_close(database);

		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
		string result = _storage.getRoutingInfo(42);
		CPPUNIT_ASSERT(result == "Hallo Welt");
    }

    void SQLiteBundleStorageTestSuite::getBundleRoutingInfoTest(){
    	int err;
    	data::Bundle bundle1, bundle2, bundle3;
    	getBundle(bundle1);
    	data::BundleID BID1(bundle1);
    	getBundle(bundle2);
    	data::BundleID BID2(bundle2);
    	getBundle(bundle3);
    	data::BundleID BID3(bundle3);
    	{
    		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
    		_storage.store(bundle1);
//    		_storage.store(bundle2);
//    		_storage.store(bundle3);
    	}

    	sqlite3 *database;
		sqlite3_stmt *set;
		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		set = prepareStatement("INSERT INTO BundleRoutingInfo (BundleID,Key,Routing) VALUES (?,?,?);",database);
		sqlite3_bind_text(set,1,BID1.toString().c_str(),BID1.toString().length(),SQLITE_TRANSIENT);
		sqlite3_bind_text(set,3,"Hallo Welt",10,SQLITE_TRANSIENT);
		sqlite3_bind_int(set,2,42);
		sqlite3_step(set);
		sqlite3_reset(set);

		sqlite3_bind_text(set,1, BID2.toString().c_str(), BID2.toString().length(),SQLITE_TRANSIENT);
		sqlite3_bind_int(set,2,12);
		sqlite3_bind_text(set,3,"error",5,SQLITE_TRANSIENT);
		err = sqlite3_step(set);
		sqlite3_reset(set);

		sqlite3_bind_text(set,1,BID1.toString().c_str(),BID1.toString().length(),SQLITE_TRANSIENT);
		sqlite3_bind_int(set,2,42);
		sqlite3_bind_text(set,3,"error",5,SQLITE_TRANSIENT);
		sqlite3_step(set);
		sqlite3_finalize(set);
		sqlite3_close(database);

		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
		string result = _storage.getBundleRoutingInfo(BID1,42);
		CPPUNIT_ASSERT(result == "Hallo Welt");
    }

    void SQLiteBundleStorageTestSuite::storeBundleRoutingInfoTest(){
    	int err,key;
    	data::Bundle bundle1, bundle2, bundle3;
    	getBundle(bundle1);
    	data::BundleID BID1(bundle1);
    	getBundle(bundle2);
    	data::BundleID BID2(bundle2);
    	getBundle(bundle3);
    	data::BundleID BID3(bundle3);
    	{
    		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
    		_storage.store(bundle1);
    		_storage.store(bundle2);
    		_storage.storeBundleRoutingInfo(BID1,23,"Hallo Welt");
    		_storage.storeBundleRoutingInfo(BID2,42,"cs.tu-bs");
    		_storage.storeBundleRoutingInfo(BID3,43,"error");
    		_storage.store(bundle3);
    		_storage.storeBundleRoutingInfo(BID3,42,"error");
    	}

    	sqlite3 *database;
		sqlite3_stmt *get;
		string bundleID, info;
		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get = prepareStatement("SELECT * FROM BundleRoutingInfo;",database);
		err = sqlite3_step(get);
		while(err == SQLITE_ROW){
			bundleID = (const char*)sqlite3_column_text(get,1);
			key		 = sqlite3_column_int(get,2);
			info	 = (const char*)sqlite3_column_text(get,3);
			CPPUNIT_ASSERT(bundleID == BID1.toString() || bundleID == BID2.toString() || bundleID == BID3.toString());
			CPPUNIT_ASSERT(info == "Hallo Welt" || info == "cs.tu-bs" || info == "error");
			err = sqlite3_step(get);
		}
		if(err != SQLITE_DONE)
			cout << "fehler" <<endl;
		sqlite3_finalize(get);
		sqlite3_close(database);
    }

    void SQLiteBundleStorageTestSuite::deleteDepricatedTest(){
    	int err;
    	sqlite3 *database;
		sqlite3_stmt *get;
    	data::Bundle bundle1, bundle2, bundle3;

    	getBundle(bundle1);
    	bundle1._timestamp = 3;
    	bundle1._lifetime = 3;
    	data::BundleID BID1(bundle1);

    	getBundle(bundle2, "dtn:bob/app42","dtn:alice/app19");
    	bundle2._timestamp = 5;
    	bundle2._lifetime = 3;
    	data::BundleID BID2(bundle2);

    	getBundle(bundle3, "dtn:trudy/app1","dtn:alice/app19");
    	bundle3._timestamp = 7;
    	bundle3._lifetime = 3;
    	data::BundleID BID3(bundle3);

    	{
    		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
    		_storage.store(bundle1);
    		_storage.store(bundle2);
    		_storage.store(bundle3);
    		core::TimeEventAction action(core::TIME_SECOND_TICK);
    		core::TimeEvent::raise(1,action);
//    		_storage.actual_time = 1;
//    		_storage.deleteexpired();
    		CPPUNIT_ASSERT(_storage.deleteList.size() == 0);
    		CPPUNIT_ASSERT(_storage.nextExpiredTime == 6);
    	}

		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get = prepareStatement("SELECT * FROM Bundles ORDER BY BundleID ASC;",database);
		err = sqlite3_step(get);
		CPPUNIT_ASSERT(err == SQLITE_ROW);
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,1) == BID1.toString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,2) == bundle1._source.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,3) == bundle1._destination.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,4) == bundle1._reportto.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,5) == bundle1._custodian.getString());
		CPPUNIT_ASSERT(sqlite3_column_int64(get,6) == bundle1._procflags);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,7) == bundle1._timestamp);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,8) == bundle1._sequencenumber);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,9) == bundle1._lifetime);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,10) == bundle1._fragmentoffset);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,11) == bundle1._appdatalength);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,12) == (bundle1._timestamp + bundle1._lifetime));
		CPPUNIT_ASSERT(sqlite3_column_int64(get,13) == 2004);

		err = sqlite3_step(get);
		CPPUNIT_ASSERT(err == SQLITE_ROW);
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,1) == BID2.toString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,2) == bundle2._source.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,3) == bundle2._destination.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,4) == bundle2._reportto.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,5) == bundle2._custodian.getString());
		CPPUNIT_ASSERT(sqlite3_column_int64(get,6) == bundle2._procflags);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,7) == bundle2._timestamp);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,8) == bundle2._sequencenumber);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,9) == bundle2._lifetime);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,10) == bundle2._fragmentoffset);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,11) == bundle2._appdatalength);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,12) == (bundle2._timestamp + bundle2._lifetime));
		CPPUNIT_ASSERT(sqlite3_column_int64(get,13) == 2004);

		err = sqlite3_step(get);
		CPPUNIT_ASSERT(err == SQLITE_ROW);
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,1) == BID3.toString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,2) == bundle3._source.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,3) == bundle3._destination.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,4) == bundle3._reportto.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,5) == bundle3._custodian.getString());
		CPPUNIT_ASSERT(sqlite3_column_int64(get,6) == bundle3._procflags);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,7) == bundle3._timestamp);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,8) == bundle3._sequencenumber);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,9) == bundle3._lifetime);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,10) == bundle3._fragmentoffset);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,11) == bundle3._appdatalength);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,12) == (bundle3._timestamp + bundle3._lifetime));
		CPPUNIT_ASSERT(sqlite3_column_int64(get,13) == 2004);
		CPPUNIT_ASSERT(sqlite3_step(get) == SQLITE_DONE);
		sqlite3_finalize(get);
		sqlite3_close(database);

    	{
    		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
    		core::TimeEventAction action(core::TIME_SECOND_TICK);
    		core::TimeEvent::raise(8,action);
//    		_storage.actual_time = 8;
//    		_storage.deleteexpired();
    		sleep(1);
    		CPPUNIT_ASSERT(_storage.deleteList.size() == 2);
    		CPPUNIT_ASSERT(_storage.nextExpiredTime == 10);
    		list<string>::iterator it;
    		for(it = _storage.deleteList.begin(); it != _storage.deleteList.end();it++){
    			cout << "Dateiname: " << (*it) << endl;
    		}
    	}

		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get = prepareStatement("SELECT * FROM Bundles ORDER BY BundleID ASC;",database);
		err = sqlite3_step(get);
		if(err == SQLITE_ROW){
			CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,1) == BID3.toString());
			CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,2) == bundle3._source.getString());
			CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,3) == bundle3._destination.getString());
			CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,4) == bundle3._reportto.getString());
			CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,5) == bundle3._custodian.getString());
			CPPUNIT_ASSERT(sqlite3_column_int64(get,6) == bundle3._procflags);
			CPPUNIT_ASSERT(sqlite3_column_int64(get,7) == bundle3._timestamp);
			CPPUNIT_ASSERT(sqlite3_column_int64(get,8) == bundle3._sequencenumber);
			CPPUNIT_ASSERT(sqlite3_column_int64(get,9) == bundle3._lifetime);
			CPPUNIT_ASSERT(sqlite3_column_int64(get,10) == bundle3._fragmentoffset);
			CPPUNIT_ASSERT(sqlite3_column_int64(get,11) == bundle3._appdatalength);
			CPPUNIT_ASSERT(sqlite3_column_int64(get,12) == (bundle3._timestamp + bundle1._lifetime));
			CPPUNIT_ASSERT(sqlite3_column_int64(get,13) == 2004);
		}
		CPPUNIT_ASSERT(sqlite3_step(get) == SQLITE_DONE);
		sqlite3_finalize(get);
		sqlite3_close(database);
    }

    void SQLiteBundleStorageTestSuite::fragmentationTest(){
    	//ToDo Fragmentierungstest anpassen, so dass auch Fragmente mit mehreren Blöcken getestet werden.
    	int err;
    	data::Bundle bundle;
    	list<data::Bundle> frags = getFragment(bundle,20,40,60);
    	list<data::Bundle>::iterator it;
    	stringstream strom;
    	data::BundleID ID(frags.front()._source,frags.front()._timestamp, frags.front()._sequencenumber);
    	{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
			for(it = frags.begin(); it!= frags.end();it++){
				_storage.storeFragment(*it);
			}
		}
    	sqlite3 *database;
		sqlite3_stmt *get, *getfrag, *getBlocks;
		string bundleID, info;
		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get = prepareStatement("SELECT * FROM Bundles;",database);
		err = sqlite3_step(get);
		CPPUNIT_ASSERT(err == SQLITE_ROW);
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,1) == ID.toString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,2) == frags.front()._source.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,3) == frags.front()._destination.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,4) == frags.front()._reportto.getString());
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(get,5) == frags.front()._custodian.getString());
		size_t procflags = frags.front()._procflags - dtn::data::Bundle::FRAGMENT;
		CPPUNIT_ASSERT(sqlite3_column_int64(get,6) == procflags);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,7) == frags.front()._timestamp);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,8) == frags.front()._sequencenumber);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,9) == frags.front()._lifetime);
		CPPUNIT_ASSERT(sqlite3_column_int64(get,12) == (frags.front()._timestamp + frags.front()._lifetime));
		sqlite3_finalize(get);
		getfrag = prepareStatement("SELECT * FROM Fragments;",database);
		CPPUNIT_ASSERT(sqlite3_step(getfrag) == SQLITE_DONE);
		sqlite3_finalize(getfrag);

		DIR *directory;
		struct dirent *dirpointer;
		if( (directory=opendir("/home/pennywise/workspace/EmmaSVN/Unittest/Fragments")) != NULL ){
			while((dirpointer=readdir(directory)) != NULL){
				if ( strcmp(dirpointer->d_name,".") && strcmp(dirpointer->d_name,"..") )
					CPPUNIT_FAIL("File in /Fragments found.");
			}
		}
		closedir(directory);

		string filename("/home/pennywise/workspace/EmmaSVN/Unittest/Block/");
		if( (directory=opendir("/home/pennywise/workspace/EmmaSVN/Unittest/Block")) != NULL ){
			for(int i=1;i<=3;i++){
				CPPUNIT_ASSERT( (dirpointer=readdir(directory)) != NULL);
				if ( strcmp(dirpointer->d_name,".") && strcmp(dirpointer->d_name,"..") ){
					filename.append((string)dirpointer->d_name);
				}
			}
			CPPUNIT_ASSERT( (dirpointer=readdir(directory)) == NULL);
		}
		closedir(directory);

		getBlocks = prepareStatement("SELECT * FROM Block;",database);
		CPPUNIT_ASSERT(sqlite3_step(getBlocks) == SQLITE_ROW);
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(getBlocks,1) == ID.toString());
		CPPUNIT_ASSERT(sqlite3_column_int(getBlocks,2) == data::PayloadBlock::BLOCK_TYPE);
		CPPUNIT_ASSERT((const char*)sqlite3_column_text(getBlocks,3) == filename);
		CPPUNIT_ASSERT(sqlite3_column_int(getBlocks,4) == 1);
		sqlite3_finalize(getBlocks);

		sqlite3_close(database);
    }

	void SQLiteBundleStorageTestSuite::getByEIDTest(){
		data::Bundle bundle1, bundle2, bundle3;
		bool exceptionthrown = false;
		string a("eins"), b("zwei");
		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
		data::EID eid1("dtn:alice/app3");
		data::EID eid2("dtn:bob/app2");

		getBundle(bundle1);
		getBundle(bundle2,"dtn:testsource", "dtn:testdestination");

		try{
			_storage.get(eid1);
		}catch(dtn::core::SQLiteBundleStorage::NoBundleFoundException){
			exceptionthrown = true;
		}
		CPPUNIT_ASSERT(exceptionthrown);
		exceptionthrown = false;

		_storage.store(bundle2);
		_storage.store(bundle1);

		try{
			_storage.get(eid1);
		}catch(dtn::core::SQLiteBundleStorage::NoBundleFoundException){
			exceptionthrown = true;
		}
		CPPUNIT_ASSERT(exceptionthrown);

		getBundle(bundle3,"dtn:bob/app2","dtn:alice/app3");
		_storage.store(bundle3);
		try{
			_storage.get(eid1);
		}catch(dtn::core::SQLiteBundleStorage::NoBundleFoundException){
			exceptionthrown = true;
		}
		CPPUNIT_ASSERT(!exceptionthrown);
	}

    void SQLiteBundleStorageTestSuite::consistenceTest(){
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
			dtn::data::Bundle bundle,resultBundle;
			vector<data::Bundle> bvector;
			for (int i = 0; i<100;i++){
				data::Bundle tmp_Bdl;
				getBundle(tmp_Bdl);
				bvector.push_back(tmp_Bdl);
				_storage.store(tmp_Bdl);
			}
		}
		int err;
		sqlite3 *database;
		sqlite3_stmt *get,*get2;
		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get = prepareStatement("DELETE FROM Bundles WHERE ROWID BETWEEN 18 AND 30;",database);
		err = sqlite3_step(get);
		sqlite3_finalize(get);
		sqlite3_close(database);
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
		}
		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get2 = prepareStatement("SELECT Count(ROWID) From Bundles WHERE ROWID NOT BETWEEN 10 AND 30;",database);
		sqlite3_step(get2);
		err = sqlite3_column_int(get2,0);
		sqlite3_finalize(get2);
		sqlite3_close(database);
		CPPUNIT_ASSERT(err == 79);
//		CPPUNIT_ASSERT(storageEventList.size() == 8 );
    }

    void SQLiteBundleStorageTestSuite::clearALLTest(){
    	{
    		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
    		_storage.clear();
    	}
    	dbInsert();
    	{
    		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
    		_storage.clearAll();
    	}
    	sqlite3 *database;
    	sqlite3_stmt *get;
    	int err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
    	if(err != SQLITE_OK){
    		throw "unable to open the database";
    	}
    	get = prepareStatement("SELECT Count(ROWID) From Bundles;",database);
    	sqlite3_step(get);
    	CPPUNIT_ASSERT(sqlite3_column_int(get,0) == 0);
    	sqlite3_finalize(get);
    	sqlite3_close(database);
    	DIR *directory;
    	bool condition1 = false, condition2 = false, condition3 = false;
    	struct dirent *dirpointer;
    	if((directory=opendir("/home/pennywise/workspace/EmmaSVN/Unittest/Bundles")) != NULL){
    		while((dirpointer=readdir(directory)) != NULL){
    			if(!(strcmp(dirpointer->d_name,".")) || !(strcmp(dirpointer->d_name, ".."))){
    				CPPUNIT_FAIL("There are still files left.");
    				cout << "name: " << dirpointer->d_name << endl;
    			}
    		}
    	}
    	closedir(directory);
    }

    void SQLiteBundleStorageTestSuite::getBlockTest(){
    	data::Bundle bundle;
    	getBundle(bundle);
    	data::BundleID ID(bundle);
    	list<data::Block> blocklist;
    	list<data::Block*> blocklist2;
    	list<data::Block*>::iterator it;
    	data::Block* pblock;
    	blocklist2 = bundle.getBlocks();
    	for (it = blocklist2.begin(); it!=blocklist2.end();it++){
    		pblock = (*it);
    	}

    	stringstream payload1, payload2;
    	{
    		dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
    		_storage.store(bundle);
    		blocklist = _storage.getBlock(ID,'1');
    	}
    	CPPUNIT_ASSERT( blocklist.front()._procflags == pblock->_procflags );
    	ibrcommon::BLOB::Reference ref = blocklist.front().getBLOB();
    	{
    		ibrcommon::MutexLock lock(ref);
    		ref.read(payload1);
    	}
    	ref = pblock->getBLOB();
    	{
    		ibrcommon::MutexLock lock(ref);
    		pblock->getBLOB().read(payload2);
    	}
    	CPPUNIT_ASSERT(payload1.str() == payload2.str());
    }

    void SQLiteBundleStorageTestSuite::removeTest(){
		data::BundleID BID,BID2;
		vector<data::Bundle> bvector;
		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
			dtn::data::Bundle bundle,bundle2;
			getBundle(bundle);
			getBundle(bundle2,"dtn:test", "dtn:ziel");
			BID = data::BundleID(bundle);
			BID2 = data::BundleID(bundle2);
			_storage.store(bundle);
			_storage.store(bundle2);
			for (int i = 0; i<10;i++){
				data::Bundle tmp_Bdl;
				getBundle(tmp_Bdl);
				bvector.push_back(tmp_Bdl);
				_storage.store(tmp_Bdl);
			}
			_storage.remove(BID);
		}
		sqlite3 *database;
		sqlite3_stmt *get;
		int err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get = prepareStatement("SELECT * FROM Bundles WHERE BundleID = ?",database);

		sqlite3_bind_text(get,1,BID.toString().c_str(),BID.toString().length(),SQLITE_TRANSIENT);
		err = sqlite3_step(get);
		sqlite3_reset(get);
		CPPUNIT_ASSERT(err==SQLITE_DONE);

		sqlite3_bind_text(get,1,BID2.toString().c_str(),BID2.toString().length(),SQLITE_TRANSIENT);
		err = sqlite3_step(get);
		sqlite3_reset(get);
		CPPUNIT_ASSERT(err==SQLITE_ROW);
		for(int i = 0;i<10;i++){
			data::BundleID id = data::BundleID(bvector[i]);
			sqlite3_bind_text(get,1,id.toString().c_str(),id.toString().length(),SQLITE_TRANSIENT);
			err = sqlite3_step(get);
			sqlite3_reset(get);
			CPPUNIT_ASSERT(err==SQLITE_ROW);
		}
		sqlite3_finalize(get);
		sqlite3_close(database);

		{
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
			_storage.remove(BID);
		}

		err = sqlite3_open_v2(("/home/pennywise/workspace/EmmaSVN/Unittest/db"),&database,SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(err != SQLITE_OK){
			throw "unable to open the database";
		}
		get = prepareStatement("SELECT * FROM Bundles WHERE BundleID = ?",database);

		sqlite3_bind_text(get,1,BID.toString().c_str(),BID.toString().length(),SQLITE_TRANSIENT);
		err = sqlite3_step(get);
		sqlite3_reset(get);
		CPPUNIT_ASSERT(err==SQLITE_DONE);

		sqlite3_bind_text(get,1,BID2.toString().c_str(),BID2.toString().length(),SQLITE_TRANSIENT);
		err = sqlite3_step(get);
		sqlite3_reset(get);
		CPPUNIT_ASSERT(err==SQLITE_ROW);
		for(int i = 0;i<10;i++){
			data::BundleID id = data::BundleID(bvector[i]);
			sqlite3_bind_text(get,1,id.toString().c_str(),id.toString().length(),SQLITE_TRANSIENT);
			err = sqlite3_step(get);
			sqlite3_reset(get);
			CPPUNIT_ASSERT(err==SQLITE_ROW);
		}
		sqlite3_finalize(get);
		sqlite3_close(database);
    }

    void SQLiteBundleStorageTestSuite::occupiedSpaceTest(){
    	CPPUNIT_ASSERT(false);
    }

//    void SQLiteBundleStorageTestSuite::storeBlocks (const data::Bundle &bundle){
//    	const list<data::Block*> blocklist(bundle.getBlocks());
//		list<data::Block*>::const_iterator it;
//		int filedescriptor(-1), blocktyp, err, blocknumber(1), storedBytes(0);
//		fstream filestream;
//		data::BundleID ID(bundle);
//
//		for(it = blocklist.begin() ;it != blocklist.end(); it++){
//			filedescriptor = -1;
//			blocktyp = (int)(*it)->getType();
//			char *blockfilename;
//
//			//create file
//			blockfilename = strdup("/home/pennywise/workspace/EmmaSVN/Unittest/bundleXXXXXX");
//			filedescriptor = mkstemp(blockfilename);
//			if (filedescriptor == -1)
//				throw ibrcommon::IOException("Could not create file.");
//			close(filedescriptor);
//
//			filestream.open(blockfilename,ios_base::out|ios::binary);
//
//			streams::BlockStreamWriter writer(filestream);
//			//serialize and write the block into the file
//			{
//				storedBytes += writer.writeBlock((*it));
//			}
//			filestream.close();
//
//			//increment blocknumber
//			blocknumber++;
//			free(blockfilename);
//		}
//    }

    void SQLiteBundleStorageTestSuite::storeBundleTimeTest(){
    	fstream lala;
		lala.open("/home/pennywise/workspace/EmmaSVN/Unittest/payload",ios::out|ios::binary);
		for (int i = 1; i<=100; i++){
			if(i<=(100/2))
				lala<< "1";
			else
				lala<<"2";
		}
		lala.close();
		ibrcommon::File file = ibrcommon::File("/home/pennywise/workspace/EmmaSVN/Unittest/payload");
		if(!file.exists()){
			cerr << "Datei existiert nicht"<<endl;
		}

		list<data::Bundle> bundleList;
		list<data::Bundle>::iterator it;

    	fstream timestream, timestream2,timestream3;
    	timestream.open("/home/pennywise/workspace/EmmaSVN/TimeMeasurements/BundlestoreErweitert.csv", ios::out);
    	timestream2.open("/home/pennywise/workspace/EmmaSVN/TimeMeasurements/BundlestorePrototyp.csv", ios::out);

		for(int i= 0; i<100;i++){
			data::Bundle bd;
			getBundle(bd, "dtn:alice/app3", "dtn:bob/app2", file);
			bundleList.push_back(bd);
		}
		for(it = bundleList.begin(); it!=bundleList.end();it++){
    		ibrcommon::TimeMeasurement time, time2;
    		time.start();
    		storeBlocks(*it);
    		time.stop();
    		timestream << time <<endl;


    		time2.start();
			char *blockfilename;
			//create file
			blockfilename = strdup("/home/pennywise/workspace/EmmaSVN/Unittest/bundleXXXXXX");
			int filedescriptor = mkstemp(blockfilename);
			if (filedescriptor == -1)
				throw ibrcommon::IOException("Could not create file.");
			close(filedescriptor);
			fstream f;
			f.open(blockfilename,ios_base::out|ios::binary);
			f << (*it);
			time2.stop();
			timestream2 << time2 <<endl;
		}
    }

    list<data::BundleID> SQLiteBundleStorageTestSuite::insertTime(int number, int size, vector<vector<double> > &result, string payloadfile){
    	fstream lala;
    	vector<double> _result;
		lala.open(payloadfile.c_str(),ios::out|ios::binary);
		for (int i = 1; i<=size; i++){
			if(i<=(size/2))
				lala<< "1";
			else
				lala<<"2";
		}
		lala.close();
		ibrcommon::File file = ibrcommon::File(payloadfile.c_str());
		if(!file.exists()){
			cerr << "Datei existiert nicht"<<endl;
		}

    	list<data::BundleID> resultID;

    	dtn::core::SQLiteBundleStorage _storage(_path,"db",42);
    	for (int i=0; i < number; i++){
    		if(i%500 == 0)
    			cout << i <<" Bundles written"<<endl;
    		data::Bundle bd;
    		getBundle(bd, "dtn:alice/app3", "dtn:bob/app2", file);
    		data::BundleID id(bd);
    		resultID.push_back(id);
    		ibrcommon::TimeMeasurement time;
    		time.start();
    		_storage.store(bd);
    		time.stop();
    		_result.push_back(time.getMilliseconds());
    	}
    	cout << "Bundle erzeugt" <<endl;
    	result.push_back(_result);
       	return resultID;
    }

    void SQLiteBundleStorageTestSuite::deleteTime(list<data::BundleID> &idList,vector<vector<double> > &result){
    	ibrcommon::TimeMeasurement time, time2;
    	vector<double> _result;
    	list<data::BundleID>::iterator it;
    	stringstream filename;

    	time2.start();
    	dtn::core::SQLiteBundleStorage _storage(_path,"db",42);

    	for(it = idList.begin(); it!=idList.end(); it++){
    		time.start();
    		_storage.remove(*it);
    		time.stop();
    		_result.push_back(time.getMilliseconds());
    	}
    	time2.stop();
    	result.push_back(_result);
    	cout << "Dauer DeleteTest: " << time2 << endl;
    }

    void SQLiteBundleStorageTestSuite::insertTimeTest(){
    	vector<vector<double> > result_insert, resul_delete;
    	for (int i = 0 ; i < 4; i++){
    		cout << "insertTest start"<<endl;
    		clearFolder(_path);
    		list<data::BundleID> idList;
        	ibrcommon::TimeMeasurement time;

			time.start();
			idList = insertTime(10000,1000000,result_insert, "/home/pennywise/workspace/EmmaSVN/TimeMeasurements/Payload_insertErweitert");
			time.stop();
			cout << "Dauer insertTest: " << time <<endl;

			cout << "deleteTest start"<<endl;
			deleteTime(idList, resul_delete);
    	}
    	cout << "Berechnung der Mittelwerte"<<endl;
    	double time_insert, time_delete;
    	fstream strom, strom2;
    	strom.open("/home/pennywise/workspace/EmmaSVN/TimeMeasurements/insertTime_Erweitert.csv",ios::out);
    	strom2.open("/home/pennywise/workspace/EmmaSVN/TimeMeasurements/deleteTimeErweitert.csv",ios::out);
    	for(int i = 0; i<result_insert[0].size(); i++){
    		time_insert = ((result_insert[0])[i] + (result_insert[1])[i] + (result_insert[2])[i] + (result_insert[3])[i])/4;
    		strom << time_insert <<endl;

    		time_delete = ((resul_delete[0])[i] + (resul_delete[1])[i] + (resul_delete[2])[i] + (resul_delete[3])[i])/4;
    		strom2 << time_delete <<endl;
    	}
    }

    void SQLiteBundleStorageTestSuite::readTimeTest(){
    	vector<double> result;
    	vector<vector<double> > tmp;
    	list<data::BundleID> idList;
    	list<data::BundleID>::iterator it;
    	ibrcommon::TimeMeasurement time, time2;
    	for(int i = 1; i < 100; i++){
			time.start();
			idList = insertTime(10,1000000,tmp,"/home/pennywise/workspace/EmmaSVN/Unittest/Payload_insertErweitert");
			fstream timestream;
			timestream.open("/home/pennywise/workspace/EmmaSVN/TimeMeasurements/readErweitert.csv", ios::out|ios::app);
			time.stop();

			time2.start();
			dtn::core::SQLiteBundleStorage _storage(_path,"db",42);

			cout.flush();
			cout << "Speicherzeit: " << time <<endl;
			for(it = idList.begin();it != idList.end();it++){
				ibrcommon::TimeMeasurement time;
				time.start();
				_storage.get(*it);
				time.stop();
				result.push_back(time.getMilliseconds());
			}
			double read_result;
			read_result = (result[0]+result[1]+result[2]+result[3]+result[4]+result[5]+result[6]+result[7]+result[8]+result[9])/10;
			timestream << read_result <<endl;
			time2.stop();
			timestream.close();
    	}
    	cout << "Auslesedauer: " << time2 <<endl;
    }

    list<std::string> SQLiteBundleStorageTestSuite::finishedThreads;
    void SQLiteBundleStorageTestSuite::signal(string name){
    	finishedThreads.push_front(name);
    }
}
}
