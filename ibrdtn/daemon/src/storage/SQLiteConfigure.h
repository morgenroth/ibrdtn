/*
 * SQLiteConfigure.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 * Written-by: Matthias Myrtus
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
