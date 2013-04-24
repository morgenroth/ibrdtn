/*
 * PrimaryBlock.h
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

#ifndef PRIMARYBLOCK_H_
#define PRIMARYBLOCK_H_

#include "ibrdtn/data/EID.h"
#include "ibrdtn/data/Dictionary.h"
#include "ibrdtn/data/Serializer.h"
#include <ibrcommon/thread/Mutex.h>
#include <string>
#include <iostream>

#ifdef SWIG
#    define DEPRECATED
#else
#    define DEPRECATED  __attribute__ ((deprecated))
#endif

namespace dtn
{
	namespace data
	{
		static const unsigned char BUNDLE_VERSION = 0x06;

		class PrimaryBlock
		{
		public:
			/**
			 * Define the Bundle Priorities
			 * PRIO_LOW low priority for this bundle
			 * PRIO_MEDIUM medium priority for this bundle
			 * PRIO_HIGH high priority for this bundle
			 */
			enum PRIORITY
			{
				PRIO_LOW = 0,
				PRIO_MEDIUM = 1,
				PRIO_HIGH = 2
			};

			enum FLAGS
			{
				FRAGMENT = 1 << 0x00,
				APPDATA_IS_ADMRECORD = 1 << 0x01,
				DONT_FRAGMENT = 1 << 0x02,
				CUSTODY_REQUESTED = 1 << 0x03,
				DESTINATION_IS_SINGLETON = 1 << 0x04,
				ACKOFAPP_REQUESTED = 1 << 0x05,
				RESERVED_6 = 1 << 0x06,
				PRIORITY_BIT1 = 1 << 0x07,
				PRIORITY_BIT2 = 1 << 0x08,
				CLASSOFSERVICE_9 = 1 << 0x09,
				CLASSOFSERVICE_10 = 1 << 0x0A,
				CLASSOFSERVICE_11 = 1 << 0x0B,
				CLASSOFSERVICE_12 = 1 << 0x0C,
				CLASSOFSERVICE_13 = 1 << 0x0D,
				REQUEST_REPORT_OF_BUNDLE_RECEPTION = 1 << 0x0E,
				REQUEST_REPORT_OF_CUSTODY_ACCEPTANCE = 1 << 0x0F,
				REQUEST_REPORT_OF_BUNDLE_FORWARDING = 1 << 0x10,
				REQUEST_REPORT_OF_BUNDLE_DELIVERY = 1 << 0x11,
				REQUEST_REPORT_OF_BUNDLE_DELETION = 1 << 0x12,
				STATUS_REPORT_REQUEST_19 = 1 << 0x13,
				STATUS_REPORT_REQUEST_20 = 1 << 0x14,

				// DTNSEC FLAGS (these are customized flags and not written down in any draft)
				DTNSEC_REQUEST_SIGN = 1 << 0x1A,
				DTNSEC_REQUEST_ENCRYPT = 1 << 0x1B,
				DTNSEC_STATUS_VERIFIED = 1 << 0x1C,
				DTNSEC_STATUS_CONFIDENTIAL = 1 << 0x1D,
				DTNSEC_STATUS_AUTHENTICATED = 1 << 0x1E,
				IBRDTN_REQUEST_COMPRESSION = 1 << 0x1F
			};

			PrimaryBlock();
			virtual ~PrimaryBlock();

			/**
			 * This method is deprecated because it does not recognize the AgeBlock
			 * as alternative age verification.
			 */
			bool isExpired() const DEPRECATED;

			std::string toString() const;

			void set(FLAGS flag, bool value);
			bool get(FLAGS flag) const;

			PRIORITY getPriority() const;
			void setPriority(PRIORITY p);

			/**
			 * relabel the primary block with a new sequence number and a timestamp
			 */
			void relabel();

			bool operator==(const PrimaryBlock& other) const;
			bool operator!=(const PrimaryBlock& other) const;
			bool operator<(const PrimaryBlock& other) const;
			bool operator>(const PrimaryBlock& other) const;

			size_t procflags;
			size_t timestamp;
			size_t sequencenumber;
			size_t lifetime;
			size_t _fragmentoffset;
			size_t _appdatalength;

			EID _source;
			EID _destination;
			EID _reportto;
			EID _custodian;

		private:
			static ibrcommon::Mutex __sequence_lock;
			static size_t __sequencenumber;
			static size_t __last_timestamp;
		};
	}
}

#endif /* PRIMARYBLOCK_H_ */
