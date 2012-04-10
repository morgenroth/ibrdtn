/*
 * RWLock.h
 *
 *  Created on: 27.03.2012
 *      Author: morgenro
 */

#ifndef RWLOCK_H_
#define RWLOCK_H_

#include "ibrcommon/thread/RWMutex.h"

namespace ibrcommon
{
	class RWLock
	{
	public:
		RWLock(RWMutex &mutex, RWMutex::LockState state);
		virtual ~RWLock();

	private:
		RWMutex &_mutex;
	};
} /* namespace ibrcommon */
#endif /* RWLOCK_H_ */
