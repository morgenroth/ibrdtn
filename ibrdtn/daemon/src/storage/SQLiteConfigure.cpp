/*
 * SQLiteConfigure.cpp
 *
 *  Created on: 30.03.2010
 *      Author: Myrtus
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
					IBRCOMMON_LOGGER(error) << "SQLite configure failed: " << err << IBRCOMMON_LOGGER_ENDL;
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
