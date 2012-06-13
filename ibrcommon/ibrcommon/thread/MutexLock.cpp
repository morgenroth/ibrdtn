/*
 * MutexLock.cpp
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
