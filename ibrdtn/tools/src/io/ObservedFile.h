/*
 * ObservedFile.h
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

#include "io/FileHash.h"
#include <ibrcommon/data/File.h>
#include <ibrcommon/refcnt_ptr.h>
#include <vector>
#include <set>
#include <string>

#ifndef OBSERVEDFILE_H_
#define OBSERVEDFILE_H_

namespace io
{
	class ObservedFile
	{
	public:
		ObservedFile(const ibrcommon::File &file);
		virtual ~ObservedFile();

		bool operator==(const ObservedFile &other) const;
		bool operator<(const ObservedFile &other) const;

		/**
		 * check if the file has changed
		 */
		void update();

		/**
		 * returns the number of update() calls this file is stable
		 */
		size_t getStableCounter() const;

		/**
		 * return the corresponding file reference
		 */
		const ibrcommon::File& getFile() const;

		/**
		 * recursive list all files in this directory
		 */
		void findFiles(std::set<ObservedFile> &files) const;

		/**
		 * Returns the hash of the last update() call
		 */
		const io::FileHash& getHash() const;

	private:
		io::FileHash __hash() const;

		static ibrcommon::File* __copy(const ibrcommon::File &file);

		refcnt_ptr<ibrcommon::File> _file;

		size_t _stable_counter;
		io::FileHash _last_hash;

		const static std::string TAG;
	};
}

#endif /* OBSERVEDFILE_H_ */
