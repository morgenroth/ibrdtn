/*
 * SDNV.h
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
 *    THIS FILE BASES ON DTN_2.4.0/SERVLIB/BUNDLING/SDNV.H
 */

#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include <stdint.h>

#ifndef _SDNV_H_
#define _SDNV_H_

/**
 * Class to handle parsing and formatting of self describing numeric
 * values (SDNVs).
 *
 * The basic idea is to enable a compact byte representation of
 * numeric values that may widely vary in size. This encoding is based
 * on the ASN.1 specification for encoding Object Identifier Arcs.
 *
 * Conceptually, the integer value to be encoded is split into 7-bit
 * segments. These are encoded into the output byte stream, such that
 * the high order bit in each byte is set to one for all bytes except
 * the last one.
 *
 * Note that this implementation only handles values up to 64-bits in
 * length (since the conversion is between a uint64_t and the encoded
 * byte sequence).
 */

namespace dtn
{
	namespace data
	{
		class SDNV
		{
		public:
			/**
			 * The maximum length for this SDNV implementation is 10 bytes,
			 * since the maximum value is 64 bits wide.
			 */
			static const size_t MAX_LENGTH = 10;

			/**
			 * Constructor for a SDNV object
			 * @param value The new value of the SDNV
			 */
			SDNV(const uint64_t value = 0);

			/**
			 * Destructor
			 * @return
			 */
			~SDNV();

			/**
			 * Determine the encoded length of the value.
			 * @return The length of the encoded value.
			 */
			size_t getLength() const;

			/**
			 * Returns the decoded value.
			 * @return The decoded value.
			 */
			uint64_t getValue() const;

			const SDNV& operator=(const uint64_t &value);

			bool operator==(const SDNV &value) const;
			bool operator!=(const SDNV &value) const;

			SDNV operator+(const SDNV &value);
			SDNV& operator+=(const SDNV &value);

			SDNV operator-(const SDNV &value);
			SDNV& operator-=(const SDNV &value);

			bool operator&(const uint64_t &value) const;

			bool operator<(const SDNV &value) const;
			bool operator<=(const SDNV &value) const;
			bool operator>(const SDNV &value) const;
			bool operator>=(const SDNV &value) const;

		private:
			friend std::ostream &operator<<(std::ostream &stream, const dtn::data::SDNV &obj);
			friend std::istream &operator>>(std::istream &stream, dtn::data::SDNV &obj);

			uint64_t _value;
		};
	}
}

#endif /* _SDNV_H_ */
