/*
 * SDNV.cpp
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
 *
 * THIS FILE BASES ON DTN_2.4.0/SERVLIB/BUNDLING/SDNV.CC
 */

using namespace std;

#include "ibrdtn/data/SDNV.h"
#include "ibrdtn/data/Exceptions.h"
#include <cstdlib>
#include <cstring>

namespace dtn
{
	namespace data
	{

		SDNV::SDNV(const uint64_t value) : _value(value)
		{}

		SDNV::~SDNV()
		{}

		size_t SDNV::getLength() const
		{
			size_t val_len = 0;
			uint64_t tmp = _value;

			do {
				tmp = tmp >> 7;
				val_len++;
			} while (tmp != 0);

			return val_len;
		}

		uint64_t SDNV::getValue() const
		{
			return _value;
		}

		const SDNV& SDNV::operator=(const uint64_t &value)
		{
			_value = value;
			return (*this);
		}

		bool SDNV::operator==(const SDNV &value) const
		{
			return (_value == value._value);
		}

		bool SDNV::operator!=(const SDNV &value) const
		{
			return (_value != value._value);
		}

		SDNV SDNV::operator+(const SDNV &value)
		{
			return SDNV(value.getValue() + getValue());
		}

		SDNV& SDNV::operator+=(const SDNV &value)
		{
			_value += value._value;
			return (*this);
		}

		SDNV SDNV::operator-(const SDNV &value)
		{
			return SDNV(value.getValue() - getValue());
		}

		SDNV& SDNV::operator-=(const SDNV &value)
		{
			_value -= value._value;
			return (*this);
		}

		bool SDNV::operator&(const uint64_t &value) const
		{
			return (_value & value);
		}

		bool SDNV::operator<(const SDNV &value) const
		{
			return _value < value._value;
		}

		bool SDNV::operator<=(const SDNV &value) const
		{
			return !(value < *this);
		}

		bool SDNV::operator>(const SDNV &value) const
		{
			return value < *this;
		}

		bool SDNV::operator>=(const SDNV &value) const
		{
			return !(*this < value);
		}

		std::ostream &operator<<(std::ostream &stream, const dtn::data::SDNV &obj)
		{
			unsigned char buffer[10];
			unsigned char *bp = &buffer[0];
			uint64_t val = obj._value;

			const size_t val_len = obj.getLength();

			if (!(val_len > 0)) throw InvalidDataException("ERROR(SDNV): !(val_len > 0)");
			if (!(val_len <= SDNV::MAX_LENGTH)) throw InvalidDataException("ERROR(SDNV): !(val_len <= MAX_LENGTH)");

			// Now advance bp to the last byte and fill it in backwards with the value bytes.
			bp += val_len;
			unsigned char high_bit = 0; // for the last octet
			do {
				--bp;
				*bp = (unsigned char)(high_bit | (val & 0x7f));
				high_bit = (1 << 7); // for all but the last octet
				val = val >> 7;
			} while (val != 0);

			if (!(bp == &buffer[0])) throw InvalidDataException("ERROR(SDNV): !(bp == buffer)");

			// write encoded value to the stream
			stream.write((const char*)&buffer[0], val_len);

			return stream;
		}

		std::istream &operator>>(std::istream &stream, dtn::data::SDNV &obj)
		{
			// TODO: check if the value fits into sizeof(size_t)

			size_t val_len = 0;
			unsigned char bp = 0;
			unsigned char start = 0;

			obj._value = 0;
			do {
				stream.get((char&)bp);

				obj._value = (obj._value << 7) | (bp & 0x7f);
				++val_len;

				if ((bp & (1 << 7)) == 0)
					break; // all done;

				if (start == 0) start = bp;
			} while (1);

			if ((val_len > SDNV::MAX_LENGTH) || ((val_len == SDNV::MAX_LENGTH) && (start != 0x81)))
				throw InvalidDataException("ERROR(SDNV): overflow value in sdnv");

			return stream;
		}
	}
}
