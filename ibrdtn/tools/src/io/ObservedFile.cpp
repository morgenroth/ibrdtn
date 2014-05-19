/*
 * ObservedFile.cpp
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: David Goltzsche <goltzsch@ibr.cs.tu-bs.de>
 *             Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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
 *  Created on: Sep 30, 2013
 */

#include "config.h"
#include "io/ObservedFile.h"
#include <string.h>
#include <sstream>
#include <typeinfo>
#include <limits>

#include <ibrcommon/ibrcommon.h>
#include <ibrcommon/Logger.h>
#ifdef IBRCOMMON_SUPPORT_SSL
#include <ibrcommon/ssl/MD5Stream.h>
#endif

#ifdef HAVE_LIBTFFS
#include "io/FATFile.h"
#endif

namespace io
{
	const std::string ObservedFile::TAG = "ObservedFile";

	ObservedFile::ObservedFile(const ibrcommon::File &file)
	 : _file(__copy(file))
	{
		update();
	}

	ObservedFile::~ObservedFile()
	{
	}

	const io::FileHash& ObservedFile::getHash() const
	{
		return _last_hash;
	}

	size_t ObservedFile::getStableCounter() const
	{
		return _stable_counter;
	}

	ibrcommon::File* ObservedFile::__copy(const ibrcommon::File &file)
	{
		// make a copy of the file object
#ifdef HAVE_LIBTFFS
		try {
			const io::FATFile &f = dynamic_cast<const io::FATFile&>(file);
			return new io::FATFile(f);
		} catch (const std::bad_cast&) { };
#endif

		return new ibrcommon::File(file);
	}

	const ibrcommon::File& ObservedFile::getFile() const
	{
		return *_file;
	}

	void ObservedFile::findFiles(std::set<ObservedFile> &files) const
	{
		if (!getFile().isDirectory()) return;

#ifdef HAVE_LIBTFFS
		try {
			const io::FATFile &f = dynamic_cast<const io::FATFile&>(*_file);

			std::list<io::FATFile> filelist;
			if (f.getFiles(filelist) == 0)
			{
				for (std::list<io::FATFile>::const_iterator it = filelist.begin(); it != filelist.end(); ++it)
				{
					const io::ObservedFile of(*it);

					if (of.getFile().isSystem()) continue;

					if (of.getFile().isDirectory()) of.findFiles(files);
					else files.insert(of);
				}
			}

			IBRCOMMON_LOGGER_TAG(TAG,notice) << "findFiles returning " << files.size() << " files" << IBRCOMMON_LOGGER_ENDL;
			return;
		} catch (const std::bad_cast&) { };
#endif

		const ibrcommon::File &f = dynamic_cast<const ibrcommon::File&>(*_file);

		std::list<ibrcommon::File> filelist;
		if (f.getFiles(filelist) == 0)
		{
			for (std::list<ibrcommon::File>::const_iterator it = filelist.begin(); it != filelist.end(); ++it)
			{
				const io::ObservedFile of(*it);

				if (of.getFile().isSystem()) continue;

				if (of.getFile().isDirectory()) of.findFiles(files);
				else files.insert(of);
			}
		}
		IBRCOMMON_LOGGER_TAG(TAG,notice) << "findFiles returning " << files.size() << " files" << IBRCOMMON_LOGGER_ENDL;
	}

	void ObservedFile::update()
	{
		const FileHash latest = __hash();

		if (_last_hash != latest) {
			_last_hash = latest;
			_stable_counter = 0;
		} else {
			// protect against overflow
			if (_stable_counter < std::numeric_limits<size_t>::max()) _stable_counter++;
		}
	}

	bool ObservedFile::operator<(const ObservedFile &other) const
	{
		return getFile() < other.getFile();
	}

	bool ObservedFile::operator==(const ObservedFile &other) const
	{
		return getFile() == other.getFile();
	}

	io::FileHash ObservedFile::__hash() const
	{
#ifdef IBRCOMMON_SUPPORT_SSL
		IBRCOMMON_LOGGER_TAG(TAG,notice) << "using MD5-algorithm from SSL, inputs:" << "\n"
										 << "\t" << "timestamp: " << _file->lastmodify() << "\n"
										 << "\t" << "filesize : " << _file->size() << "\n"
										 << "\t" << "filepath : " << _file->getPath() << "\n"
										 << IBRCOMMON_LOGGER_ENDL;
		// update hash
		ibrcommon::MD5Stream md5;
		md5 << _file->lastmodify() << "|" << _file->size() << "|" << _file->getPath() << std::flush;

		const std::string hash = ibrcommon::HashStream::extract(md5);

		return FileHash(_file->getPath(), hash);
#else
		std::stringstream ss;
		ss << _file->lastmodify() << "|" << _file->size() << "|" << _file->getPath();
		IBRCOMMON_LOGGER_TAG(TAG,notice) << "not using MD5-algorithm from SSL, inputs:" << "\n"
										 << "\t" << "timestamp: " << _file->lastmodify() << "\n"
										 << "\t" << "filesize : " << _file->size() << "\n"
										 << "\t" << "filepath : " << _file->getPath() << "\n"
										 << IBRCOMMON_LOGGER_ENDL;
		return FileHash(_file->getPath(), ss.str());
#endif

	}
}
