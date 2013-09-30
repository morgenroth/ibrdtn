/*
 * ObservedFATFile.h
 *
 *  Created on: Sep 30, 2013
 *      Author: goltzsch
 */

#include "ObservedFile.h"
#include "FATFile.h"

#ifndef OBSERVEDFATFILE_H_
#define OBSERVEDFATFILE_H_

class ObservedFATFile: public ObservedFile
{
public:
	ObservedFATFile(string path);
	virtual ~ObservedFATFile();

	virtual int getFiles(list<ObservedFile*> files);
	virtual string getPath();
	virtual bool exists();
	virtual string getBasename();
	virtual size_t size();
	virtual bool isSystem();
	virtual bool isDirectory();
	virtual unsigned char* getHash();

private:
	FATFile _file;
};

#endif /* OBSERVEDFATFILE_H_ */
