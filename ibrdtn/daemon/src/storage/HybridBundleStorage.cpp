/*
 * HybridBundleStorage.cpp
 *
 * Copyright (C) 2015 Johannes Morgenroth
 *
 * Written-by: Johannes Morgenroth <jm@m-network.de>
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

#include "storage/HybridBundleStorage.h"

namespace dtn
{
	namespace storage
	{
		const std::string HybridBundleStorage::TAG = "HybridBundleStorage";

		HybridBLOBProvider::HybridBLOBProvider(const ibrcommon::File &path, std::streamsize threshold)
		 : _path(path), _threshold(threshold)
		{

		}

		HybridBLOBProvider::~HybridBLOBProvider()
		{

		}

		ibrcommon::BLOB::Reference HybridBLOBProvider::create()
		{
			return HybridBLOB::create(_path, _threshold);
		}

		HybridBLOB::streambuf::streambuf(const ibrcommon::File &path, std::streamsize threshold)
		 : _streambuf(NULL), _file(NULL), _path(path), _threshold(threshold)
		{

		}

		HybridBLOB::streambuf::~streambuf()
		{
			if (_file)
			{
				// delete temporary file
				_file->remove();
				delete _file;
			}
		}

		int HybridBLOB::streambuf::sync()
		{
			if (pptr() != pbase()) return overflow(EOF);
			return 0;
		}

		std::streampos HybridBLOB::streambuf::seekpos(std::streampos pos, std::ios_base::openmode m)
		{
			return seekoff(std::streamoff(pos), std::ios_base::beg, m);
		}

		std::streampos HybridBLOB::streambuf::seekoff(std::streamoff off, std::ios_base::seekdir seekdir, std::ios_base::openmode)
		{
			if (sync()) return -1;
			if (_streambuf == NULL) {
				setg(0,0,0);
				return -1;
			}

			return -1; //_streambuf->seekoff(off, seekdir);
		}

		void HybridBLOB::streambuf::clear()
		{
			if (_streambuf) {
				delete _streambuf;
				_streambuf = new std::stringbuf();
			}

			if (_file) {
				delete _file;
				_file = NULL;
			}
		}

		void HybridBLOB::streambuf::open()
		{
			if (_file)
			{
				ibrcommon::BLOB::_filelimit.wait();

				std::filebuf *fb = static_cast<std::filebuf*>(_streambuf);
				fb->open(_file->getPath().c_str(), ios::in|ios::binary);

				if (!fb->is_open())
				{
					// Release semaphore on failed file open
					ibrcommon::BLOB::_filelimit.post();

					throw ibrcommon::CanNotOpenFileException(*_file);
				}
			}
		}

		void HybridBLOB::streambuf::close()
		{
			if (_file)
			{
				static_cast<std::filebuf*>(_streambuf)->close();

				ibrcommon::BLOB::_filelimit.post();
			}
		}

		std::streamsize HybridBLOB::streambuf::size()
		{
			if (_file) {
				return _file->size();
			} else {
				return static_cast<std::stringbuf*>(_streambuf)->str().length();
			}
		}

		HybridBLOB::HybridBLOB(const ibrcommon::File &path, std::streamsize threshold)
		 : _streambuf(path, threshold), _stream(&_streambuf)
		{
		}

		HybridBLOB::~HybridBLOB()
		{
		}

		ibrcommon::BLOB::Reference HybridBLOB::create(const ibrcommon::File &path, std::streamsize threshold)
		{
			return ibrcommon::BLOB::Reference(new HybridBLOB(path, threshold));
		}

		void HybridBLOB::clear()
		{
			_streambuf.clear();
		}

		void HybridBLOB::open()
		{
			_streambuf.open();
		}

		void HybridBLOB::close()
		{
			_streambuf.close();
		}

		std::streamsize HybridBLOB::__get_size()
		{
			return _streambuf.size();
		}

		HybridBundleStorage::HybridBundleStorage(const ibrcommon::File &workdir, const dtn::data::Length maxsize, const unsigned int buffer_limit)
		 : dtn::storage::BundleStorage(maxsize), _path(workdir), _buffer_limit(buffer_limit)
		{
		}

		HybridBundleStorage::~HybridBundleStorage()
		{

		}

		const std::string HybridBundleStorage::getName() const
		{
			return TAG;
		}

		void HybridBundleStorage::componentUp() throw ()
		{

		}

		void HybridBundleStorage::componentDown() throw ()
		{

		}

		void HybridBundleStorage::store(const dtn::data::Bundle &bundle)
		{

		}

		bool HybridBundleStorage::contains(const dtn::data::BundleID &id)
		{
			return false;
		}

		dtn::data::MetaBundle HybridBundleStorage::info(const dtn::data::BundleID &id)
		{
			return dtn::data::MetaBundle();
		}

		dtn::data::Bundle HybridBundleStorage::get(const dtn::data::BundleID &id)
		{
			return dtn::data::Bundle();
		}

		void HybridBundleStorage::get(const BundleSelector &cb, BundleResult &result) throw (NoBundleFoundException, BundleSelectorException)
		{

		}

		const BundleSeeker::eid_set HybridBundleStorage::getDistinctDestinations()
		{
			return BundleSeeker::eid_set();
		}

		void HybridBundleStorage::remove(const dtn::data::BundleID &id)
		{

		}

		void HybridBundleStorage::releaseCustody(const dtn::data::EID &custodian, const dtn::data::BundleID &id)
		{

		}
	} /* namespace storage */
} /* namespace dtn */
