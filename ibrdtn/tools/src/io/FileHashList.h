/*
 * FileHash.cpp
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
 *  Created on: Oct 17, 2013
 */
#ifndef FILEHASHLIST_H_
#define FILEHASHLIST_H_

#include <list>
#include <string>
#include "FileHash.h"
#include "ObservedFile.h"

class FileHashList {
public:
	FileHashList();
	virtual ~FileHashList();
	void add(FileHash fh);
	void removeAll(std::string hash);
	bool contains(ObservedFile* of);
private:
	std::list<FileHash> _list;
};

#endif /* FILEHASHLIST_H_ */
