/*
 * StringBundle.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef STRINGBUNDLE_H_
#define STRINGBUNDLE_H_

#include "ibrdtn/api/Bundle.h"
#include <ibrcommon/data/BLOB.h>

namespace dtn
{
	namespace api
	{
		/**
		 * This class could be used to send small string data (all have to fit into
		 * the system RAM) through the bundle protocol.
		 */
		class StringBundle : public dtn::api::Bundle
		{
		public:
			/**
			 * Constructor of the StringBundle object. It just needs a destination
			 * as parameter.
			 * @param destination The desired destination of the bundle.
			 */
			StringBundle(const dtn::data::EID &destination);

			/**
			 * Destricutor of the StringBundle object.
			 */
			virtual ~StringBundle();

			/**
			 * Append a string to the payload.
			 */
			void append(const std::string &data);

		private:
			// reference to the BLOB where all data is stored until transmission
			ibrcommon::BLOB::Reference _ref;
		};
	}
}

#endif /* STRINGBUNDLE_H_ */
