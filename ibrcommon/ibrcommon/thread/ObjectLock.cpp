/*
 * ObjectLock.cpp
 *
 *  Created on: 23.11.2010
 *      Author: morgenro
 */

#include "ibrcommon/thread/ObjectLock.h"
#include "ibrcommon/thread/MutexLock.h"

namespace ibrcommon
{
	ibrcommon::Conditional ObjectLock::__cond;
	std::set<void*> ObjectLock::__locks;

	ObjectLock::ObjectLock(void *obj)
	 : _obj(obj)
	{
		ibrcommon::MutexLock l(__cond);

		// check for lock
		while (__locks.find(obj) != __locks.end())
		{
			// wait while there is a lock
			__cond.wait();
		}

		// add lock
		__locks.insert(obj);
	}

	ObjectLock::~ObjectLock()
	{
		ibrcommon::MutexLock l(__cond);
		__locks.erase(_obj);
	}
}
