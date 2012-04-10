/*
 * FileBundle.h
 *
 *  Created on: 24.07.2009
 *      Author: morgenro
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
