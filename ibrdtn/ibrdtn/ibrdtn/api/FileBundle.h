/*
 * FileBundle.h
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

#ifndef FILEBUNDLE_H_
#define FILEBUNDLE_H_

#include "ibrdtn/api/Bundle.h"
#include <ibrcommon/data/File.h>

namespace dtn
{
	namespace api
	{
		/**
		 * This class could be used to send whole files through the
		 * bundle protocol. The file is not copied before sending and thus it
		 * has to be available until the hole bundle is sent to the daemon.
		 */
		class FileBundle : public dtn::api::Bundle
		{
		public:
			/**
			 * Constructor of the FileBundle object. It needs a destination and
			 * a file object which points to an existing file.
			 * @param destination The destination EID for the bundle.
			 * @param file The file to send.
			 */
			FileBundle(const dtn::data::EID &destination, const ibrcommon::File &file);

			/**
			 * Destructor of the FileBundle object.
			 */
			virtual ~FileBundle();
		};
	}
}

#endif /* FILEBUNDLE_H_ */
