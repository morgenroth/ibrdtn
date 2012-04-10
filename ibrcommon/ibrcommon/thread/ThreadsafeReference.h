#ifndef THREADSAFEREFERENCE_H_
#define THREADSAFEREFERENCE_H_

#include "MutexLock.h"

namespace ibrcommon {

	template <class T>
	class ThreadsafeReference
	{
	public:
		ThreadsafeReference(T& ref, MutexInterface& mutex)
			: _item(ref), _l(mutex)
		{}
		T& operator*() const
		{
			return _item;
		}
	private:
		T& _item;
		MutexLock _l;
	};

} // namespace ibrcommon

#endif // THREADSAFEREFERENCE_H_
