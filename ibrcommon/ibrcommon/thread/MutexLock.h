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
