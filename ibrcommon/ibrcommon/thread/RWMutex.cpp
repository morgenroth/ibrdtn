/*
 * RWMutex.cpp
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

#include "ibrcommon/thread/RWMutex.h"
#include <errno.h>

namespace ibrcommon
{
	RWMutex::RWMutex()
	{
		pthread_rwlock_init(&_rwlock, NULL);
	}

	RWMutex::~RWMutex()
	{
		pthread_rwlock_destroy(&_rwlock);
	}

	void RWMutex::trylock(LockState state) throw (MutexException)
	{
		int ret = 0;

		switch (state) {
		case LOCK_READONLY:
			ret = pthread_rwlock_tryrdlock(&_rwlock);
			break;

		case LOCK_READWRITE:
			ret = pthread_rwlock_trywrlock(&_rwlock);
			break;
		}

		switch (ret)
		{
		case 0:
			break;

		case EBUSY:
			throw MutexException("The mutex could not be acquired because it was already locked.");
			break;
		}
	}

	void RWMutex::enter(LockState state) throw (MutexException)
	{
		switch (state) {
		case LOCK_READONLY:
			pthread_rwlock_rdlock(&_rwlock);
			break;

		case LOCK_READWRITE:
			pthread_rwlock_wrlock(&_rwlock);
			break;
		}
	}

	void RWMutex::leave() throw (MutexException)
	{
		pthread_rwlock_unlock(&_rwlock);
	}
} /* namespace ibrcommon */
