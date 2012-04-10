/*
 * SQLiteConfigure.h
 *
 *  Created on: 30.03.2010
 *      Author: Myrtus
 */

#ifndef SQLITECONFIGURE_H_
#define SQLITECONFIGURE_H_
#include "ibrcommon/thread/Mutex.h"

namespace dtn{
namespace storage{
	class SQLiteConfigure{
	public:
		static void configure();
		static void shutdown();

	private:
		SQLiteConfigure(){};
		static ibrcommon::Mutex _mutex;
		static bool _isSet;
	};
}
}
#endif /* SQLITECONFIGURE_H_ */
