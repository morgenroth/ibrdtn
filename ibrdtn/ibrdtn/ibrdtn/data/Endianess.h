/*
 * endian.c
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

// DO NOT INCLUDE THIS FILE IN A HEADER FILE!

#include <stdint.h>

#ifndef ENDIANESS_H_
#define ENDIANESS_H_

namespace dtn
{
	namespace data
	{
		uint16_t bswap16(uint16_t x);
		uint32_t bswap32(uint32_t x);
		uint64_t bswap64(uint64_t x);
	}
}

#ifdef HAVE_GLIB
	// GLIB provides optimized macros for endianess conversion
	// we prefer to use them
	#include <glib.h>
#else
	// include endianess headers
	#if HAVE_ENDIAN_H
		// include standard POSIX endian.h
		#include <endian.h>
	#elif HAVE_MACHINE_ENDIAN_H
		// include BSD endian.h
		#include <machine/endian.h>
	#elif __WIN32__
		// windows has no endian header, but
		// provides byte swapping in the winsock2 API
		#include <winsock2.h>
		#define BYTE_ORDER __BYTE_ORDER
		#define LITTLE_ENDIAN __LITTLE_ENDIAN
	#endif

	// check if system byte order is network order
	#if BYTE_ORDER == LITTLE_ENDIAN
		// if POSIX htobe16 is available use it
		#ifdef htobe16
		#define GUINT16_TO_BE(x) htobe16(x)
		#else
		#define GUINT16_TO_BE(x) dtn::data::bswap16(x)
		#endif

		// if POSIX htobe32 is available use it
		#ifdef htobe32
		#define GUINT32_TO_BE(x) htobe32(x)
		#else
		#define GUINT32_TO_BE(x) dtn::data::bswap32(x)
		#endif

		// if POSIX htobe64 is available use it
		#ifdef htobe64
		#define GUINT64_TO_BE(x) htobe64(x)
		#else
		#define GUINT64_TO_BE(x) dtn::data::bswap64(x)
		#endif
	#else
		// system byte order is equal to network byte order
		// generate dummy macros
		#define GUINT16_TO_BE(x) (x)
		#define GUINT32_TO_BE(x) (x)
		#define GUINT64_TO_BE(x) (x)
	#endif
#endif

#endif /* ENDIANESS_H_ */
