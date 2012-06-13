/*
 * MutexLock.h
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

#ifndef IBRCOMMON_MUTEXLOCK_H_
#define IBRCOMMON_MUTEXLOCK_H_

#include "ibrcommon/thread/Mutex.h"

namespace ibrcommon
{
	class MutexLock
	{
		public:
			MutexLock(MutexInterface &m);
			virtual ~MutexLock();

		private:
			MutexInterface &m_mutex;
	};

	class MutexTryLock
	{
		public:
			MutexTryLock(MutexInterface &m);
			virtual ~MutexTryLock();

		private:
			MutexInterface &m_mutex;
	};

	class IndicatingLock : public MutexLock
	{
		public:
			IndicatingLock(MutexInterface &m, bool &indicator);
			virtual ~IndicatingLock();

		private:
			bool &_indicator;
	};
}

#endif /*IBRCOMMON_MUTEXLOCK_H_*/
