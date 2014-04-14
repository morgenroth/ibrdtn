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
 *
 *  This class provides the same methods as ibrcommon::File, but for a file
 *  on a FAT image.
 *  A few methods are not implemented, because not needed;
 */

#ifndef FATFILE_H_
#define FATFILE_H_

#include "config.h"
#include <ibrcommon/data/File.h>
#include <list>

namespace io
{
	class FatImageReader;

	class FATFile : public ibrcommon::File
	{
	public:
		FATFile(const FatImageReader &reader);
		FATFile(const FatImageReader &reader, const std::string &file_path);
		virtual ~FATFile();

		int getFiles(std::list<FATFile> &files) const;

		virtual int remove(bool recursive);

		FATFile get(const std::string &filename) const;
		FATFile getParent() const;

		virtual bool exists() const;
		virtual void update();
		virtual size_t size() const;
		virtual time_t lastaccess() const;
		virtual time_t lastmodify() const;
		virtual time_t laststatchange() const;

		const FatImageReader &getReader() const;

	private:
		const FatImageReader &_reader;
		const static std::string TAG;
	};
}
#endif /* FATFILE_H_ */
