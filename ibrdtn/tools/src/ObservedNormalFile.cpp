/*
 * ObservedNormalFile.cpp
 *
 *  Created on: Sep 30, 2013
 *      Author: goltzsch
 */

#include "ObservedNormalFile.h"

ObservedNormalFile::ObservedNormalFile(string path) : _file(path),_last_sent(0)
{
	// TODO Auto-generated constructor stub

}

ObservedNormalFile::~ObservedNormalFile()
{
	// TODO Auto-generated destructor stub
}

int ObservedNormalFile::getFiles( list<ObservedFile*> files )
{

	list<ibrcommon::File> normalfiles;
	list<ibrcommon::File>::iterator ff_iter;
	int ret = _file.getFiles(normalfiles);
	for(ff_iter = normalfiles.begin(); ff_iter != normalfiles.end(); ff_iter++)
	{
		if((*ff_iter).isDirectory())
		{
			(*ff_iter).getFiles(normalfiles);
		}
		else
		{
			const ObservedFile* of = new ObservedFATFile((*ff_iter).getPath());
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

bool ObservedNormalFile::isDirectory()
{
	return _file.isDirectory();
}

time_t ObservedNormalFile::getLastTimestamp()
{
	return max(_file.lastmodify(), _file.laststatchange());
}

void ObservedNormalFile::addSize()
{
	_sizes.push_back(_file.size());
}

bool ObservedNormalFile::operator ==( ObservedFile* other )
{
	return (other->getPath() == getPath());
}
