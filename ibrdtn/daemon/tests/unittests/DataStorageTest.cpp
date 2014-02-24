/*
 * DataStorageTest.cpp
 *
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

#include "DataStorageTest.h"
#include <ibrdtn/data/Bundle.h>
#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/thread/Conditional.h>
#include <map>

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cerrno>

CPPUNIT_TEST_SUITE_REGISTRATION(DataStorageTest);

void DataStorageTest::testHashTest()
{
	class DataContainer : public dtn::storage::DataStorage::Container
	{
	public:
		DataContainer() {};
		~DataContainer() {};

		std::string getId() const
		{
			return "5b3132333435363738392e31325d2064746e3a2f2f746573742f746573746572";
		}

		std::ostream& serialize(std::ostream &stream)
		{
			stream << "data";
			return stream;
		}
	};

	DataCallbackDummy callback;
	ibrcommon::File datapath("/tmp/datastorage");
	dtn::storage::DataStorage storage(callback, datapath, true);

	dtn::storage::DataStorage::Hash h = storage.store(new DataContainer());

	CPPUNIT_ASSERT_EQUAL(std::string("5b3132333435363738392e31325d2064746e3a2f2f746573742f746573746572"), h.value);
}

void DataStorageTest::testStoreTest()
{
	class DataCallback : public dtn::storage::DataStorage::Callback
	{
	public:
		DataCallback() : stored(false) {};
		virtual ~DataCallback() {};

		void wait()
		{
			ibrcommon::MutexLock l(_cond);
			if (stored) return;
			_cond.wait();
		}

		void eventDataStorageStored(const dtn::storage::DataStorage::Hash&)
		{
			ibrcommon::MutexLock l(_cond);
			stored = true;
			_cond.signal(true);
		};

		void eventDataStorageStoreFailed(const dtn::storage::DataStorage::Hash&, const ibrcommon::Exception&) {};
		void eventDataStorageRemoved(const dtn::storage::DataStorage::Hash&) {};
		void eventDataStorageRemoveFailed(const dtn::storage::DataStorage::Hash&, const ibrcommon::Exception&) {};
		void iterateDataStorage(const dtn::storage::DataStorage::Hash&, dtn::storage::DataStorage::istream&) {};

		bool stored;
		ibrcommon::Conditional _cond;
	};

	class DataContainer : public dtn::storage::DataStorage::Container
	{
	public:
		DataContainer() {};
		~DataContainer() {};

		std::string getId() const
		{
			return "5b3132333435363738392e31325d2064746e3a2f2f746573742f746573746572";
		}

		std::ostream& serialize(std::ostream &stream)
		{
			stream << "data";
			return stream;
		}
	};

	DataCallback callback;
	ibrcommon::File datapath("/tmp/datastorage");
	dtn::storage::DataStorage storage(callback, datapath, true);

	// startup the storage
	storage.start();

	// store some data in the storage
	dtn::storage::DataStorage::Hash h = storage.store(new DataContainer());

	// wait until all data is stored
	callback.wait();

	// get the data
	dtn::storage::DataStorage::istream s = storage.retrieve(h);

	// copy the data
	std::stringstream ss; ss << (*s).rdbuf();

	// check the data
	CPPUNIT_ASSERT_EQUAL(std::string("data"), ss.str());
}

void DataStorageTest::testRemoveTest()
{
	class DataCallback : public dtn::storage::DataStorage::Callback
	{
	public:
		DataCallback() : stored(false) {};
		virtual ~DataCallback() {};

		void wait()
		{
			ibrcommon::MutexLock l(_cond);
			if (stored) return;
			_cond.wait();
			stored = false;
		}

		void eventDataStorageStored(const dtn::storage::DataStorage::Hash&)
		{
			ibrcommon::MutexLock l(_cond);
			stored = true;
			_cond.signal(true);
		};

		void eventDataStorageStoreFailed(const dtn::storage::DataStorage::Hash&, const ibrcommon::Exception&) {};
		void eventDataStorageRemoved(const dtn::storage::DataStorage::Hash&)
		{
			ibrcommon::MutexLock l(_cond);
			stored = true;
			_cond.signal(true);
		};

		void eventDataStorageRemoveFailed(const dtn::storage::DataStorage::Hash&, const ibrcommon::Exception&) {};
		void iterateDataStorage(const dtn::storage::DataStorage::Hash&, dtn::storage::DataStorage::istream&) {};

		bool stored;
		ibrcommon::Conditional _cond;
	};

	class DataContainer : public dtn::storage::DataStorage::Container
	{
	public:
		DataContainer() {};
		~DataContainer() {};

		std::string getId() const
		{
			return "5b3132333435363738392e31325d2064746e3a2f2f746573742f746573746572";
		}

		std::ostream& serialize(std::ostream &stream)
		{
			stream << "data";
			return stream;
		}
	};

	DataCallback callback;
	ibrcommon::File datapath("/tmp/datastorage");
	dtn::storage::DataStorage storage(callback, datapath, true);

	// startup the storage
	storage.start();

	// store some data in the storage
	dtn::storage::DataStorage::Hash h = storage.store(new DataContainer());

	// wait until all data is stored
	callback.wait();

	// get the data
	CPPUNIT_ASSERT_NO_THROW(storage.retrieve(h));

	// delete the data
	storage.remove(h);

	// wait until all data is removed
	callback.wait();

	// the data should be gone
	CPPUNIT_ASSERT_THROW(storage.retrieve(h), dtn::storage::DataStorage::DataNotAvailableException);
}

void DataStorageTest::testStressTest()
{
	class DataCallback : public dtn::storage::DataStorage::Callback
	{
	public:
		DataCallback(std::list<dtn::storage::DataStorage::Hash> &data)
		: stored(0), failed(0), _data(data) {};
		virtual ~DataCallback() {};

		void wait(int value)
		{
			ibrcommon::MutexLock l(_cond);
			if ((stored + failed) == value) return;
			_cond.wait();
		}

		void eventDataStorageStored(const dtn::storage::DataStorage::Hash &hash)
		{
			ibrcommon::MutexLock l(_cond);
			stored++;
			_data.push_back(hash);
			_cond.signal(true);
		};

		void eventDataStorageStoreFailed(const dtn::storage::DataStorage::Hash&, const ibrcommon::Exception &ex)
		{
			ibrcommon::MutexLock l(_cond);
			std::cout << "store failed: " << ex.what() << std::endl;
			failed++;
			_cond.signal(true);
		};
		void eventDataStorageRemoved(const dtn::storage::DataStorage::Hash&) {};
		void eventDataStorageRemoveFailed(const dtn::storage::DataStorage::Hash&, const ibrcommon::Exception&)
		{
			std::cout << "remove failed" << std::endl;
		};
		void iterateDataStorage(const dtn::storage::DataStorage::Hash&, dtn::storage::DataStorage::istream&) {};

		int stored;
		int failed;
		ibrcommon::Conditional _cond;
		std::list<dtn::storage::DataStorage::Hash> &_data;
	};

	class DataContainer : public dtn::storage::DataStorage::Container
	{
	public:
		DataContainer(size_t id, size_t bytes, const std::string &data)
		 : _id(id), _bytes(bytes), _data(data)
		{ };

		~DataContainer() {};

		std::string getId() const
		{
			std::stringstream ss; ss << "datastorage-" << _id;
			return ss.str();
		}

		std::ostream& serialize(std::ostream &stream)
		{
			dtn::data::Bundle fake;
			fake.source = dtn::data::EID("dtn://test1/fake");
			fake.destination = dtn::data::EID("dtn://test/fake");

			ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();

			{
				ibrcommon::BLOB::iostream io = ref.iostream();

				// write some test data into the blob
				for (unsigned int j = 0; j < _bytes; ++j)
				{
						(*io) << _data;
				}
			}

			fake.push_back(ref);

			// get an serializer for bundles
			dtn::data::DefaultSerializer s(stream);

			// length of the bundle
			unsigned int size = s.getLength(fake);

			// serialize the bundle
			s << fake; stream.flush();

			// check the streams health
			if (!stream.good())
			{
				std::stringstream ss; ss << "Output stream went bad [" << std::strerror(errno) << "]";
				throw dtn::SerializationFailedException(ss.str());
			}

			// get the write position
			if (size > stream.tellp())
			{
				std::stringstream ss; ss << "Not all data were written [" << stream.tellp() << " of " << size << " bytes]";
				throw dtn::SerializationFailedException(ss.str());
			}

			return stream;
		}

		size_t _id;
		size_t _bytes;
		const std::string _data;
	};

	std::list<dtn::storage::DataStorage::Hash> _data;
	DataCallback callback(_data);
	ibrcommon::File datapath("/tmp/datastorage");
	dtn::storage::DataStorage storage(callback, datapath, true);
	storage.start();

	std::map<dtn::storage::DataStorage::Hash, size_t> _datavolume;

	const int num_of_sizes = 12;
	size_t sizes[num_of_sizes] = { 1, 5, 10, 50, 100, 500, 1000, 5000, 10000, 50000, 100000, 500000 };
	size_t id = 0;
	std::string testdata = "0123456789";

	for (int i = 0; i < num_of_sizes; ++i)
	{
		for (int j = 0; j < 100; ++j)
		{
			dtn::storage::DataStorage::Hash h = storage.store(new DataContainer(id, sizes[i], testdata));
			_datavolume[h] = sizes[i];
			id++;
		}
	}

	callback.wait(num_of_sizes * 100);

	// now check all files
	CPPUNIT_ASSERT_EQUAL(0, callback.failed);

//	for (std::list<dtn::storage::DataStorage::Hash>::const_iterator iter = _data.begin();
//			iter != _data.end(); ++iter)
//	{
//		const dtn::storage::DataStorage::Hash &h = (*iter);
//		size_t size = _datavolume[h];
//
//		dtn::storage::DataStorage::istream stream = storage.retrieve(h);
//
//		for (unsigned int i = 0; i < size; ++i)
//		{
//			for (unsigned int k = 0; k < testdata.length(); ++k)
//			{
//				char v = (*stream).get();
//				char e = testdata.c_str()[k];
//				if (e != v)
//				{
//					std::cerr << "ERROR: wrong letter found in stream. " << " expected: " << e << ", found: " << v << std::endl;
//					CPPUNIT_ASSERT_EQUAL(e, v);
//				}
//			}
//		}
//	}
}

void DataStorageTest::setUp()
{
	// create temporary directory for data storage
	ibrcommon::File datapath("/tmp/datastorage");
	ibrcommon::File::createDirectory(datapath);
}

void DataStorageTest::tearDown()
{
	// delete the temporary directory for data storage
	ibrcommon::File datapath("/tmp/datastorage");
	datapath.remove(true);
}
