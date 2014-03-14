/*
 * TarUtils.h
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: David Goltzsche <goltzsch@ibr.cs.tu-bs.de>
 *             Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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
 *  Created on: Sep 16, 2013
 */

#ifndef TARUTILS_H_
#define TARUTILS_H_
#include "config.h"
#include "io/ObservedFile.h"
#include <ibrcommon/data/File.h>
#include <set>

namespace io
{
	class TarUtils
	{
	public:
		TarUtils();
		virtual ~TarUtils();

		/**
		 * write tar archive to payload block, FATFile version
		 */
		static void write( std::ostream &output, const io::ObservedFile &root, const std::set<ObservedFile> &files_to_send );

		/*
		 * read tar archive from payload block, write to file
		 */
		static void read( const ibrcommon::File &extract_folder, std::istream &input );

	private:
		static std::string rel_filename(const ObservedFile &parent, const ObservedFile&);
	};
}

#endif /* TARUTILS_H_ */
