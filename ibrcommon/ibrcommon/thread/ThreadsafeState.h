/*
 * ThreadsafeState.h
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

#ifndef THREADSAFESTATE_H_
#define THREADSAFESTATE_H_

#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/thread/Conditional.h>

namespace ibrcommon
{
	template <class T>
	class ThreadsafeState : protected ibrcommon::Conditional
	{
	protected:
		T _state;
		const T _final_state;

	public:
		ThreadsafeState(T init, T final)
		 : _state(init), _final_state(final)
		{
		};

		virtual ~ThreadsafeState()
		{
			ibrcommon::MutexLock l(*this);
			_state = _final_state;
			this->signal(true);
		};

		void reset(T init)
		{
			_state = init;
		}

		T get() const
		{
			return _state;
		}

		void wait(size_t st)
		{
			ibrcommon::MutexLock l(*this);
			while (!(_state & st))
			{
				if (_state == _final_state) return;
				ibrcommon::Conditional::wait();
			}
		}

		T operator=(T st)
		{
			// return, if the final state is reached
			if (_state == _final_state) return _state;

			ibrcommon::MutexLock l(*this);
			_state = st;
			this->signal(true);
			return _state;
		}

		bool operator==(T st) const
		{
			return (st == _state);
		}

		bool operator!=(T st) const
		{
			return (st != _state);
		}

		class Locked
		{
		private:
			ThreadsafeState<T> &_tss;
			MutexLock _lock;

		public:
			Locked(ThreadsafeState<T> &tss)
			 : _tss(tss), _lock(tss)
			{
			};

			virtual ~Locked()
			{
				_tss.signal(true);
			};

			T get() const
			{
				return _tss._state;
			}

			T operator=(T st)
			{
				// return, if the final state is reached
				if (_tss._state == _tss._final_state) return _tss._state;

				_tss._state = st;
				return _tss._state;
			}

			bool operator==(T st)
			{
				return (_tss._state == st);
			}

			bool operator!=(T st)
			{
				return (_tss._state != st);
			}

			void wait(size_t st)
			{
				while (!(_tss._state & st))
				{
					if (_tss._state == _tss._final_state) return;
					((Conditional&)_tss).wait();
				}
			}

			void wait(T st)
			{
				while (_tss._state != st)
				{
					if (_tss._state == _tss._final_state) return;
					((Conditional&)_tss).wait();
				}
			}
		};

		Locked lock()
		{
			return *this;
		}
	};
}

#endif /* THREADSAFESTATE_H_ */
