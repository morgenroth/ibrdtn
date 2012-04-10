/*
 * RWMutex.h
 *
 *  Created on: 27.03.2012
 *      Author: morgenro
 */

#ifndef RWMUTEX_H_
#define RWMUTEX_H_

#include "ibrcommon/thread/Mutex.h"
#include <pthread.h>

namespace ibrcommon
{
	class RWMutex
	{
	public:
		enum LockState {
			LOCK_READONLY = 0,
			LOCK_READWRITE = 1
		};

		RWMutex();
		virtual ~RWMutex();

		virtual void trylock(LockState state) throw (MutexException);
		virtual void enter(LockState state) throw (MutexException);
		virtual void leave() throw (MutexException);

	protected:
		pthread_rwlock_t _rwlock;
		//pthread_mutexattr_t m_attr;
	};
} /* namespace ibrcommon */
#endif /* RWMUTEX_H_ */
