/*
 * AtomicCounter.cpp
 *
 *  Created on: 12.02.2010
 *      Author: morgenro
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
