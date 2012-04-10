/*
 * RWMutex.cpp
 *
 *  Created on: 27.03.2012
 *      Author: morgenro
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
