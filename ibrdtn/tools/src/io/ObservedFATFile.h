/*
 * ObservedFATFile.h
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

#ifdef HAVE_LIBTFFS
#include "ObservedFile.h"
#include "FATFile.h"

#ifndef OBSERVEDFATFILE_H_
#define OBSERVEDFATFILE_H_

class ObservedFATFile : public ObservedFile
{
public:
	ObservedFATFile(const std::string& file_path);
	virtual ~ObservedFATFile();

	virtual int getFiles(std::list<ObservedFile*>& files);
	virtual std::string getPath();
	virtual bool exists();
	virtual std::string getBasename();
	virtual void update();

private:
	FATFile _file;
};

#endif /* OBSERVEDFATFILE_H_ */
#endif /* HAVE_LIBTFFS*/
