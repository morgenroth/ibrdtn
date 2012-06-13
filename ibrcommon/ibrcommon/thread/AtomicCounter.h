/*
 * AtomicCounter.h
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
