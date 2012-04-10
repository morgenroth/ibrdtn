/*
 * AtomicCounter.h
 *
 *  Created on: 12.02.2010
 *      Author: morgenro
 */

#ifndef ATOMICCOUNTER_H_
#define ATOMICCOUNTER_H_

#include "ibrcommon/thread/Conditional.h"

namespace ibrcommon
{
	class AtomicCounter {
	public:
		AtomicCounter(int init = 0);
		virtual ~AtomicCounter();

		int value();
		void wait(int until = 0);

		void unblockAll();

		AtomicCounter& operator++();
		AtomicCounter operator++(int);

		AtomicCounter& operator--();
		AtomicCounter operator--(int);

		class Lock
		{
		public:
			Lock(AtomicCounter &counter);
			virtual ~Lock();

		private:
			AtomicCounter &_counter;
		};

	private:
		ibrcommon::Conditional _lock;
		int _value;
		bool _unblock;

	};
}


#endif /* ATOMICCOUNTER_H_ */
