/*
 * ObservedNormalFile.cpp
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: David Goltzsche <goltzsch@ibr.cs.tu-bs.de>
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
#include "ObservedNormalFile.h"
#include <ibrdtn/config.h>
#include <sstream>

#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#endif

ObservedNormalFile::ObservedNormalFile(const std::string& path) : _file(path)
{
}

ObservedNormalFile::~ObservedNormalFile()
{
}

int ObservedNormalFile::getFiles(list<ObservedFile*>& files)
{
	list<ibrcommon::File> normalfiles;
	list<ibrcommon::File>::iterator nf_iter;
	int ret = _file.getFiles(normalfiles);
	for(nf_iter = normalfiles.begin(); nf_iter != normalfiles.end(); nf_iter++)
	{
		if((*nf_iter).isSystem())
			continue;

		if((*nf_iter).isDirectory())
			(*nf_iter).getFiles(normalfiles);
		else
		{
			ObservedFile* of = new ObservedNormalFile((*nf_iter).getPath());
			files.push_back(of);
		}
	}
	return ret;

}

std::string ObservedNormalFile::getPath()
{
	return _file.getPath();
}

bool ObservedNormalFile::exists()
{
	return _file.exists();
}

std::string ObservedNormalFile::getBasename()
{
	return _file.getBasename();
}

void ObservedNormalFile::update()
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
	_hash = std::string(hash);
}
