/*
 * FATFile.h
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
 *  Created on: Sep 23, 2013
 */

#include "io/FATFile.h"
#include "io/FatImageReader.h"
#include <ibrcommon/Logger.h>

namespace io
{
	const std::string FATFile::TAG = "FatFile";

	FATFile::FATFile(const FatImageReader &reader)
	 : ibrcommon::File("/", DT_DIR), _reader(reader)
	{
	}

	FATFile::FATFile(const FatImageReader &reader, const std::string &file_path)
	 : ibrcommon::File(file_path), _reader(reader)
	{
		update();
	}

	FATFile::~FATFile()
	{
	}

	int FATFile::getFiles(std::list<FATFile> &files) const
	{
		try {
			_reader.list(*this, files);
		} catch (const FatImageReader::FatImageException &e) {
			return e.getErrorCode();
		}
		IBRCOMMON_LOGGER_TAG(TAG,notice) << "getFile returning " << files.size() << " files" << IBRCOMMON_LOGGER_ENDL;
		return 0;
	}

	int FATFile::remove(bool recursive)
	{
		// deletion is not supported
		return 1;
	}

	FATFile FATFile::get(const std::string &filename) const
	{
		const ibrcommon::File child = ibrcommon::File::get(filename);
		return FATFile(_reader, child.getPath());
	}

	FATFile FATFile::getParent() const
	{
		const ibrcommon::File parent = ibrcommon::File::getParent();
		return FATFile(_reader, parent.getPath());
	}

	bool FATFile::exists() const
	{
		return _reader.exists(*this);
	}

	void FATFile::update()
	{
		if (_reader.isDirectory(*this)) {
			_type = DT_DIR;
		} else {
			_type = DT_REG;
		}
	}

	size_t FATFile::size() const
	{
		return _reader.size(*this);
	}

	time_t FATFile::lastaccess() const
	{
		return _reader.lastaccess(*this);
	}

	time_t FATFile::lastmodify() const
	{
		return lastaccess();
	}

	time_t FATFile::laststatchange() const
	{
		return lastaccess();
	}

	const FatImageReader& FATFile::getReader() const
	{
		return _reader;
	}
}
