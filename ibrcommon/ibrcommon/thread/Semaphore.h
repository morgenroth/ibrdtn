#ifndef IBRCOMMON_SEMAPHORE_H_
#define IBRCOMMON_SEMAPHORE_H_

#ifdef HAVE_SYS_SEMAPHORE_H
#include <sys/semaphore.h>
#else
#include <semaphore.h>
#endif

namespace ibrcommon
{
	class Semaphore
	{
		public:
			Semaphore(unsigned int value = 0);
			virtual ~Semaphore();

			void wait();
			void post();

		private:
			sem_t count_sem;
	};
}
#endif /*SEMAPHORE_H_*/
