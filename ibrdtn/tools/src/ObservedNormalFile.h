/*
 * ObservedNormalFile.h
 *
 *  Created on: Sep 30, 2013
 *      Author: goltzsch
 */

#ifndef OBSERVEDNORMALFILE_H_
#define OBSERVEDNORMALFILE_H_

#include "ObservedFile.h"

class ObservedNormalFile: public ObservedFile
{
public:
	ObservedNormalFile(string path);
	virtual ~ObservedNormalFile();

	virtual int getFiles(list<ObservedFile*>& files);
	virtual string getPath();
	virtual bool exists();
	virtual string getBasename();
	virtual size_t size();
	virtual bool isSystem();
	virtual bool isDirectory();

	virtual string getHash();

private:
	ibrcommon::File _file;
};

#endif /* OBSERVEDNORMALFILE_H_ */
