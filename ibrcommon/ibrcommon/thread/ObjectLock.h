/*
 * ObjectLock.h
 *
 *  Created on: 23.11.2010
 *      Author: morgenro
 */

#include <set>
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/Conditional.h>

#ifndef OBJECTLOCK_H_
#define OBJECTLOCK_H_

namespace ibrcommon
{
	class ObjectLock
	{
	public:
		ObjectLock(void *obj);
		virtual ~ObjectLock();

	protected:
		static ibrcommon::Conditional __cond;
		static std::set<void*> __locks;
		void *_obj;
	};
}

#endif /* OBJECTLOCK_H_ */
