/*
 * ObjectLock.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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
