/*
 * FileHashList.cpp
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
#include "FileHashList.h"
#include <string.h>



FileHashList::FileHashList() {
}

FileHashList::~FileHashList() {
}

void FileHashList::add(FileHash fh)
{
	_list.push_back(fh);
}

void FileHashList::removeAll(std::string hash)
{
	std::list<FileHash>::iterator iter;
	for(iter = _list.begin();iter != _list.end(); iter++)
	{
		if((*iter).getHash() == hash)
			_list.erase(iter);
	}

}

bool FileHashList::contains(ObservedFile* of)
{
	std::list<FileHash>::iterator iter;
	for(iter = _list.begin();iter != _list.end(); iter++)
	{
		const char* hash1 = of->getHash().c_str();
		const char* hash2 = (*iter).getHash().c_str();
		if(memcmp(hash1,hash2,sizeof(hash1)) == 0)
			return true;
	}
	return false;
}

