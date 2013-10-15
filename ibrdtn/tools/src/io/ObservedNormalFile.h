/*
 * ObservedNormalFile.h
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

#include "ObservedFile.h"
#include "ibrcommon/data/File.h"

#ifndef OBSERVEDNORMALFILE_H_
#define OBSERVEDNORMALFILE_H_

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
