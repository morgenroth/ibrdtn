/*
 * Iterator.h
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

#ifndef IBRCOMMON_ITERATOR_H_
#define IBRCOMMON_ITERATOR_H_

#include <iterator>
#include <algorithm>

namespace ibrcommon
{
	template<class _ForwardIterator, class _Tp>
	class find_iterator : public _ForwardIterator
	{
		_ForwardIterator _next;
		_Tp _match;

	public:
		find_iterator(_ForwardIterator begin, const _Tp& __value)
		 : _ForwardIterator(begin), _next(begin), _match(__value)
		{
		}

		bool next(_ForwardIterator end)
		{
			if (_next == end) return false;

			((_ForwardIterator&)*this) = std::find(_next, end, _match);

			_next = ((_ForwardIterator&)*this);

			if (_next == end)
			{
				return false;
			}
			else
			{
				_next++;
				return true;
			}
		}
	};
}

#endif /*IBRCOMMON_ITERATOR_H_*/
