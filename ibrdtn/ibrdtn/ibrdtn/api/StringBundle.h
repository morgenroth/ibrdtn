/*
 * StringBundle.h
 *
 *  Created on: 24.07.2009
 *      Author: morgenro
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
