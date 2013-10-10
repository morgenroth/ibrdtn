/*
 * ObservedFATFile.cpp
 *
 *  Created on: Sep 30, 2013
 *      Author: goltzsch
 */

#include <string.h>
#include <sstream>
#include <stdlib.h>
#include "ObservedFATFile.h"
#include "FATFile.cpp"

#define HAVE_OPENSSL 1 //TODO
#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#endif

ObservedFATFile::ObservedFATFile(string file_path) : ObservedFile(),_file(file_path,_conf_imgpath)
{
}

ObservedFATFile::~ObservedFATFile()
{
}

int ObservedFATFile::getFiles( list<ObservedFile*>& files )
{

	list<FATFile> fatfiles;
	list<FATFile>::iterator ff_iter;
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

string ObservedFATFile::getPath()
{
	return _file.getPath();
}

bool ObservedFATFile::exists()
{
	return _file.exists();
}

string ObservedFATFile::getBasename()
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

string ObservedFATFile::getHash()
{
	stringstream ss;
	ss << getPath() << _file.lastmodify() << _file.size();
	string toHash = ss.str();
	unsigned char hash[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)toHash.c_str(), toHash.length(), hash);
	return string((char*)hash);
}
