/*
 * ObservedFATFile.cpp
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
#include "ObservedFATFile.h"

#include <string.h>
#include <sstream>
#include <stdlib.h>

#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#endif

namespace io
{
	ObservedFATFile::ObservedFATFile(const ibrcommon::File &image_file, const std::string& file_path)
	 : ObservedFile(), _file(image_file, file_path), _image_file(image_file)
	{
	}

	ObservedFATFile::~ObservedFATFile()
	{
	}

	const ibrcommon::File& ObservedFATFile::getImageFile() const
	{
		return _image_file;
	}

	int ObservedFATFile::getFiles( std::list<ObservedFile*>& files )
	{

		std::list<FATFile> fatfiles;
		std::list<FATFile>::iterator ff_iter;
		int ret = _file.getFiles(fatfiles);
		for(ff_iter = fatfiles.begin(); ff_iter != fatfiles.end(); ff_iter++)
		{
			if((*ff_iter).isSystem())
				continue;

			if((*ff_iter).isDirectory())
				(*ff_iter).getFiles(fatfiles);
			else
			{
				ObservedFile* of = new ObservedFATFile(_file.getImageFile(), (*ff_iter).getPath());
				files.push_back(of);
			}
		}
		return ret;
	}

	std::string ObservedFATFile::getPath() const
	{
		return _file.getPath();
	}

	bool ObservedFATFile::exists()
	{
		return _file.exists();
	}

	std::string ObservedFATFile::getBasename() const
	{
		return _file.getBasename();
	}

	void ObservedFATFile::update()
	{
		//update size
		_size = _file.size();

		//update isSystem
		_is_system = _file.isSystem();

		//update isDirectory
		_is_directory = _file.isDirectory();

		//update hash
		std::stringstream ss;
		ss << getPath() << _file.lastmodify() << _file.size();
		std::string toHash = ss.str();
		char hash[MD5_DIGEST_LENGTH];
		MD5((unsigned char*)toHash.c_str(), toHash.length(), (unsigned char*) hash);
		_hash = FileHash(_file.getPath(), hash);
	}
}
