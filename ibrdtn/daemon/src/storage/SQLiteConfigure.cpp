/*
 * SQLiteConfigure.cpp
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
#include "storage/SQLiteConfigure.h"
#include <sqlite3.h>
#include <iostream>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace storage
	{
		ibrcommon::Mutex SQLiteConfigure::_mutex;
		bool SQLiteConfigure::_isSet = false;

		void SQLiteConfigure::configure()
		{
			//Configure SQLite Library
			ibrcommon::MutexLock lock = ibrcommon::MutexLock(_mutex);

			if (!_isSet)
			{
				int err = sqlite3_config(SQLITE_CONFIG_SERIALIZED);

				if (err != SQLITE_OK)
				{
					IBRCOMMON_LOGGER_TAG("SQLiteConfigure", error) << "SQLite configure failed: " << err << IBRCOMMON_LOGGER_ENDL;
					throw ibrcommon::Exception("unable to set serialized sqlite configuration");
				}

				_isSet = true;

				// initialize sqlite
				sqlite3_initialize();
			}
		}

		void SQLiteConfigure::shutdown()
		{
			sqlite3_shutdown();
		}
	}
}
