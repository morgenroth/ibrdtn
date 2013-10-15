/*
 * ObservedFATFile.cpp
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

#include <string.h>
#include <sstream>
#include <stdlib.h>
#include "ObservedFATFile.h"

#include "ibrdtn/config.h"
#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#endif

ObservedFATFile::ObservedFATFile(std::string file_path) : ObservedFile(),_file(file_path,_conf_imgpath)
{
}

ObservedFATFile::~ObservedFATFile()
{
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
			ObservedFile* of = new ObservedFATFile((*ff_iter).getPath());
			files.push_back(of);
		}
	}
	return ret;
}

std::string ObservedFATFile::getPath()
{
	return _file.getPath();
}

bool ObservedFATFile::exists()
{
	return _file.exists();
}

std::string ObservedFATFile::getBasename()
{
	return _file.getBasename();
}

size_t ObservedFATFile::size()
{
	return _file.size();
}

bool ObservedFATFile::isSystem()
{
	return _file.isSystem();
}
bool ObservedFATFile::isDirectory()
{
	return _file.isDirectory();
}

std::string ObservedFATFile::getHash()
{
	std::stringstream ss;
	ss << getPath() << _file.lastmodify() << _file.size();
	std::string toHash = ss.str();
	unsigned char hash[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)toHash.c_str(), toHash.length(), hash);
	return std::string((char*)hash);
}
