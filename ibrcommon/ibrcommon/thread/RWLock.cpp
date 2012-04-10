/*
 * RWLock.cpp
 *
 *  Created on: 27.03.2012
 *      Author: morgenro
 */

#include "ibrcommon/thread/RWLock.h"

namespace ibrcommon
{
	RWLock::RWLock(RWMutex &mutex, RWMutex::LockState state)
	 : _mutex(mutex)
	{
		_mutex.enter(state);
	}

	RWLock::~RWLock()
	{
		_mutex.leave();
	}
} /* namespace ibrcommon */
