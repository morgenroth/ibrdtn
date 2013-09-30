/*
 * ObservedFATFile.cpp
 *
 *  Created on: Sep 30, 2013
 *      Author: goltzsch
 */

#include <string.h>
#include <stdlib.h>
#include "ObservedFATFile.h"
#include "FATFile.cpp"

#define HAVE_OPENSSL 1 //TODO
#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#endif

ObservedFATFile::ObservedFATFile(string path) : ObservedFile(),_file(path)
{
	cout << "konstr of: " << _conf_imgpath<< endl;
}

ObservedFATFile::~ObservedFATFile()
{
}

int ObservedFATFile::getFiles( list<ObservedFile*> files )
{

	list<FATFile> fatfiles;
	list<FATFile>::iterator ff_iter;
	int ret = _file.getFiles(fatfiles);
	for(ff_iter = fatfiles.begin(); ff_iter != fatfiles.end(); ff_iter++)
	{
		if((*ff_iter).isDirectory())
		{
			(*ff_iter).getFiles(fatfiles);
		}
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

unsigned char* ObservedFATFile::getHash()
{
	string path = getPath();
	time_t stamp = _file.lastmodify();
	size_t size = _file.size();
	unsigned char mchar[sizeof(getPath()) + sizeof(stamp) + sizeof(size)];
	memcpy(mchar,&path,sizeof(path));
	memcpy(mchar+sizeof(path)+1,&stamp,sizeof(stamp));
	memcpy(mchar+sizeof(path)+sizeof(size)+1,&size,sizeof(&size));
	unsigned char * md = (unsigned char*) malloc(MD5_DIGEST_LENGTH);
	MD5(mchar,sizeof(mchar),md);
	return md;
}
