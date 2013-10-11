/*
 * ObservedNormalFile.cpp
 *
 *  Created on: Sep 30, 2013
 *      Author: goltzsch
 */

#include "ObservedNormalFile.h"

#define HAVE_OPENSSL 1 //TODO
#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#endif

ObservedNormalFile::ObservedNormalFile(string path) : _file(path)
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

string ObservedNormalFile::getPath()
{
	return _file.getPath();
}

bool ObservedNormalFile::exists()
{
	return _file.exists();
}

string ObservedNormalFile::getBasename()
{
	return _file.getBasename();
}

size_t ObservedNormalFile::size()
{
	return _file.size();
}

bool ObservedNormalFile::isSystem()
{
	return _file.isSystem();
}
bool ObservedNormalFile::isDirectory()
{
	return _file.isDirectory();
}

string ObservedNormalFile::getHash()
{
	stringstream ss;
	ss << getPath() << _file.lastmodify() << _file.size();
	string toHash = ss.str();
	unsigned char hash[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)toHash.c_str(), toHash.length(), hash);
	return string((char*)hash);
}
