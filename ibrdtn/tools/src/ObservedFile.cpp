/*
 * ObservedFile.cpp
 *
 *  Created on: Sep 30, 2013
 *      Author: goltzsch
 */

#include "ObservedFile.h"
#include <stdio.h>

string ObservedFile::_conf_imgpath = "";
size_t ObservedFile::_conf_rounds = 0;
bool ObservedFile::_conf_badclock = false;


ObservedFile::ObservedFile() : _last_sent(0)
{
}
ObservedFile::~ObservedFile()
{
}

void ObservedFile::setConfigBadclock( bool badclock )
{
}

bool ObservedFile::lastHashesEqual( size_t n )
{
	if (n > _hashes.size()) return false;

	for (size_t i = 1; i <= n; i++)
	{
		if (_hashes.at(_hashes.size() - i) != _hashes.at(_hashes.size() - i - 1)) return false;
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
