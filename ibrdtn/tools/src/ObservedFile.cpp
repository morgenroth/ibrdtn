/*
 * ObservedFile.cpp
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

string ObservedFile::_conf_imgpath = "";
size_t ObservedFile::_conf_rounds = 0;

ObservedFile::ObservedFile() : _last_sent(0)
{
}
ObservedFile::~ObservedFile()
{
}

bool ObservedFile::lastHashesEqual( size_t n )
{
	if (n > _hashes.size()) return false;

	for (size_t i = 1; i < n; i++)
	{
		string hash1 = _hashes.at(_hashes.size() - i);
		string hash2 = _hashes.at(_hashes.size() - i - 1);
		if(hash1 != hash2)
			return false;
	}
	return true;
}

void ObservedFile::setConfigImgPath( string path )
{
	_conf_imgpath = path;
}

void ObservedFile::setConfigRounds( size_t rounds )
{
	_conf_rounds = rounds;
}

void ObservedFile::tick()
{
	_hashes.push_back(getHash());
}

void ObservedFile::send()
{
	_hashes.clear();
}

bool ObservedFile::compare( ObservedFile* a, ObservedFile* b )
{
	bool eq =  (a->getHash() == b->getHash());
	return !eq;
}
