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

#include <ibrcommon/Exceptions.h>

#include <sstream>
#include <sys/types.h>
#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <limits>
#include <cstdlib>

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
		class ValueOutOfRangeException : public ibrcommon::Exception
		{
		public:
			ValueOutOfRangeException(const std::string &what = "The value is out of range.") throw() : ibrcommon::Exception(what)
			{
			};
		};

		template<class E>
		class SDNV
		{
		public:
			/**
			 * The maximum length for this SDNV implementation is 10 bytes,
			 * since the maximum value is 64 bits wide.
			 */
			static const size_t MAX_LENGTH = 10;

			SDNV(const double value) : _value(0) {
				E max_val = std::numeric_limits<E>::max();
				if (value > static_cast<double>(max_val))
					throw ValueOutOfRangeException();

				_value = static_cast<double>(value);
			}

			/**
			 * Constructor for a SDNV object
			 * @param value The new value of the SDNV
			 */
			template<typename T>
			SDNV(const T value = 0) : _value(0) {
				if (sizeof(T) == sizeof(_value))
				{
					_value = static_cast<T>(value);
				}
				else
				{
					E max_val = std::numeric_limits<E>::max();
					if (value > static_cast<T>(max_val))
						throw ValueOutOfRangeException();

					_value = static_cast<E>(value);
				}
			}

			SDNV() : _value(0) {
			}

			/**
			 * Destructor
			 * @return
			 */
			~SDNV() {
			}

			/**
			 * Determine the encoded length of the value.
			 * @return The length of the encoded value.
			 */
			size_t getLength() const
			{
				size_t val_len = 0;
				E tmp = _value;

				do {
					tmp = tmp >> 7;
					val_len++;
				} while (tmp != 0);

				return val_len;
			}

			template<typename T>
			T get() const {
				if (sizeof(T) == sizeof(E))
				{
					return static_cast<T>(_value);
				}
				else
				{
					T max_val = std::numeric_limits<T>::max();
					if (_value > static_cast<E>(max_val))
						throw ValueOutOfRangeException();

					return static_cast<T>(_value);
				}
			}

			template<typename T>
			const SDNV& operator=(const T &value) {
				if (sizeof(T) == sizeof(_value))
				{
					_value = value;
				}
				else
				{
					E max_val = std::numeric_limits<E>::max();
					if (value > static_cast<T>(max_val))
						throw ValueOutOfRangeException();

					_value = static_cast<E>(value);
				}

				return (*this);
			}

			bool operator==(const SDNV<E> &value) const
			{
				return (value._value == _value);
			}

			bool operator!=(const SDNV<E> &value) const
			{
				return (value._value != _value);
			}

			template<typename T>
			SDNV<E> operator+(const T &value)
			{
				E result = _value + static_cast<E>(value);
				return SDNV<E>(result);
			}

			SDNV<E> operator+(const SDNV<E> &value)
			{
				E result = _value + value._value;
				return SDNV<E>(result);
			}

			template<typename T>
			friend SDNV<E> operator+(const SDNV<E> &left, const T &right)
			{
				SDNV<E> ret(left);
				ret += right;
				return ret;
			}

			template<typename T>
			SDNV<E>& operator+=(const T &value)
			{
				_value += static_cast<E>(value);
				return (*this);
			}

			SDNV<E>& operator+=(const SDNV<E> &value)
			{
				_value += value._value;
				return (*this);
			}

			SDNV<E>& operator++() // prefix
			{
				++_value;
				return (*this);
			}

			SDNV<E> operator++(int) // postfix
			{
				SDNV<E> prev(*this);
				++_value;
				return prev;
			}

			template<typename T>
			SDNV<E> operator-(const T &value)
			{
				E result = _value - static_cast<E>(value);
				return SDNV<E>(result);
			}

			SDNV<E> operator-(const SDNV<E> &value)
			{
				E result = _value - value._value;
				return SDNV<E>(result);
			}

			template<typename T>
			friend SDNV<E> operator-(const SDNV<E> &left, const T &right)
			{
				SDNV<E> ret(left);
				ret -= right;
				return ret;
			}

			template<typename T>
			SDNV<E>& operator-=(const T &value)
			{
				_value -= static_cast<E>(value);
				return (*this);
			}

			SDNV<E>& operator-=(const SDNV<E> &value)
			{
				_value -= value._value;
				return (*this);
			}

			SDNV<E>& operator--() // prefix
			{
				--_value;
				return (*this);
			}

			SDNV<E> operator--(int) // postfix
			{
				SDNV<E> prev(*this);
				--_value;
				return prev;
			}

			template<typename T>
			SDNV<E> operator/(const T &value)
			{
				E result = _value / static_cast<E>(value);
				return SDNV<E>(result);
			}

			SDNV<E> operator/(const SDNV<E> &value)
			{
				E result = _value / value._value;
				return SDNV<E>(result);
			}

			template<typename T>
			friend SDNV<E> operator/(const SDNV<E> &left, const T &right)
			{
				SDNV<E> ret(left);
				ret /= right;
				return ret;
			}

			template<typename T>
			SDNV<E>& operator/=(const T &value)
			{
				_value /= static_cast<E>(value);
				return (*this);
			}

			SDNV<E>& operator/=(const SDNV<E> &value)
			{
				_value /= value._value;
				return (*this);
			}

			template<typename T>
			SDNV<E> operator*(const T &value)
			{
				E result = _value * static_cast<E>(value);
				return SDNV<E>(result);
			}

			SDNV<E> operator*(const SDNV<E> &value)
			{
				E result = _value * value._value;
				return SDNV<E>(result);
			}

			template<typename T>
			friend SDNV<E> operator*(const SDNV<E> &left, const T &right)
			{
				SDNV<E> ret(left);
				ret *= right;
				return ret;
			}

			template<typename T>
			SDNV<E>& operator*=(const T &value)
			{
				_value *= static_cast<E>(value);
				return (*this);
			}

			SDNV<E>& operator*=(const SDNV<E> &value)
			{
				_value *= value._value;
				return (*this);
			}

			bool operator&(const SDNV<E> &value) const
			{
				return (value._value & _value);
			}

			bool operator|(const SDNV<E> &value) const
			{
				return (value._value | _value);
			}

			SDNV<E>& operator&=(const SDNV<E> &value)
			{
				_value &= value._value;
				return (*this);
			}

			SDNV<E>& operator|=(const SDNV<E> &value)
			{
				_value |= value._value;
				return (*this);
			}

			bool operator<(const SDNV<E> &value) const
			{
				return (_value < value._value);
			}

			bool operator<=(const SDNV<E> &value) const
			{
				return (_value <= value._value);
			}

			bool operator>(const SDNV<E> &value) const
			{
				return (_value > value._value);
			}

			bool operator>=(const SDNV<E> &value) const
			{
				return (_value >= value._value);
			}

			static SDNV<E> max()
			{
				return std::numeric_limits<E>::max();
			}

			SDNV<E>& random()
			{
				// for compatibility use 32-bit here
				uint32_t val = static_cast<uint32_t>(::random());
				(*this) = static_cast<E>(val);
				return (*this);
			}

			std::string toString() const {
				std::stringstream ss;
				ss << _value;
				return ss.str();
			}

		private:
			friend std::ostream &operator<<(std::ostream &stream, const dtn::data::SDNV<E> &obj)
			{
				unsigned char buffer[10];
				unsigned char *bp = &buffer[0];
				uint64_t val = obj._value;

				const size_t val_len = obj.getLength();

				if (!(val_len > 0)) throw ValueOutOfRangeException("ERROR(SDNV): !(val_len > 0)");
				if (!(val_len <= SDNV::MAX_LENGTH)) throw ValueOutOfRangeException("ERROR(SDNV): !(val_len <= MAX_LENGTH)");

				// Now advance bp to the last byte and fill it in backwards with the value bytes.
				bp += val_len;
				unsigned char high_bit = 0; // for the last octet
				do {
					--bp;
					*bp = (unsigned char)(high_bit | (val & 0x7f));
					high_bit = (1 << 7); // for all but the last octet
					val = val >> 7;
				} while (val != 0);

				if (!(bp == &buffer[0])) throw ValueOutOfRangeException("ERROR(SDNV): !(bp == buffer)");

				// write encoded value to the stream
				stream.write((const char*)&buffer[0], val_len);

				return stream;
			}

			friend std::istream &operator>>(std::istream &stream, dtn::data::SDNV<E> &obj)
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
					throw ValueOutOfRangeException("ERROR(SDNV): overflow value in sdnv");

				return stream;
			}

			E _value;
		};
	}
}

#endif /* _SDNV_H_ */
