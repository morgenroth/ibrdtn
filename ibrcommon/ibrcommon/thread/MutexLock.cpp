#include "ibrcommon/config.h"
#include "ibrcommon/thread/MutexLock.h"

namespace ibrcommon
{
	MutexLock::MutexLock(MutexInterface &m) : m_mutex(m)
	{
		m_mutex.enter();
	}

	MutexLock::~MutexLock()
	{
		m_mutex.leave();
	}

	MutexTryLock::MutexTryLock(MutexInterface &m) : m_mutex(m)
	{
		m_mutex.trylock();
	}

	MutexTryLock::~MutexTryLock()
	{
		m_mutex.leave();
	}

	IndicatingLock::IndicatingLock(MutexInterface &m, bool &indicator)
	 : MutexLock(m), _indicator(indicator)
	{
		_indicator = true;
	}

	IndicatingLock::~IndicatingLock()
	{
		_indicator = false;
	}
}
