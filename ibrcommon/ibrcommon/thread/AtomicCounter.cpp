/*
 * AtomicCounter.cpp
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

#include "ibrcommon/thread/AtomicCounter.h"
#include "ibrcommon/thread/MutexLock.h"

namespace ibrcommon
{
	AtomicCounter::AtomicCounter(int init)
	 : _value(init), _unblock(false)
	{

	}

	AtomicCounter::~AtomicCounter()
	{
		unblockAll();
	}

	int AtomicCounter::value()
	{
		ibrcommon::MutexLock l(_lock);
		return _value;
	}

	void AtomicCounter::unblockAll()
	{
		ibrcommon::MutexLock l(_lock);
		_unblock = true;
		_lock.signal(true);
	}

	void AtomicCounter::wait(int until)
	{
		ibrcommon::MutexLock l(_lock);
		while ((_value != until) && !_unblock)
		{
			_lock.wait();
		}
	}

	AtomicCounter& AtomicCounter::operator++()
	{
		ibrcommon::MutexLock l(_lock);
		_value++;
		_lock.signal(true);
		return *this;
	}

	AtomicCounter AtomicCounter::operator++(int)
	{
		ibrcommon::MutexLock l(_lock);
		_value++;
		_lock.signal(true);
		return AtomicCounter(_value - 1);
	}

	AtomicCounter& AtomicCounter::operator--()
	{
		ibrcommon::MutexLock l(_lock);
		_value--;
		_lock.signal(true);
		return *this;
	}

	AtomicCounter AtomicCounter::operator--(int)
	{
		ibrcommon::MutexLock l(_lock);
		_value--;
		_lock.signal(true);
		return AtomicCounter(_value + 1);
	}

	AtomicCounter::Lock::Lock(AtomicCounter &counter)
	 : _counter(counter)
	{
		_counter++;
	}

	AtomicCounter::Lock::~Lock()
	{
		_counter--;
	}
}
