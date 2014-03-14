/*
 * FileHash.h
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
 *  Created on: Oct 17, 2013
 */
#ifndef FILEHASH_H_
#define FILEHASH_H_

#include <string>

namespace io
{
	class FileHash
	{
	public:
		FileHash();
		FileHash(const std::string &path, const std::string &hash);
		virtual ~FileHash();

		const std::string& getHash() const;
		const std::string& getPath() const;

		bool operator==(const FileHash& other) const;
		bool operator!=(const FileHash& other) const;
		bool operator<(const FileHash& other) const;

	private:
		std::string _path;
		std::string _hash;
	};
}

#endif /* FILEHASH_H_ */
