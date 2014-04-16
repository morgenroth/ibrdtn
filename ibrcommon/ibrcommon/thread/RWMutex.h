/*
 * RWMutex.h
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

#ifndef RWMUTEX_H_
#define RWMUTEX_H_

#include "ibrcommon/thread/Mutex.h"
#include <pthread.h>

namespace ibrcommon
{
	class RWMutex : public MutexInterface
	{
	public:
		RWMutex();
		virtual ~RWMutex();

		virtual void trylock() throw (MutexException);
		virtual void enter() throw (MutexException);
		virtual void leave() throw (MutexException);

		virtual void trylock_wr() throw (MutexException);
		virtual void enter_wr() throw (MutexException);

	protected:
		pthread_rwlock_t _rwlock;
	};
} /* namespace ibrcommon */
#endif /* RWMUTEX_H_ */
