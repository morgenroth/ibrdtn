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

#ifndef IBRDTN_ENDIAN_C_
#define IBRDTN_ENDIAN_C_

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
			uint16_t GUINT16_TO_BE(uint16_t x) {
				union { uint16_t u16; uint8_t v[2]; } ret;
				ret.v[0] = (uint8_t)(x >> 8);
				ret.v[1] = (uint8_t) x;
				return ret.u16;
			}
		#endif

		// if POSIX htobe32 is available use it
		#ifdef htobe32
		#define GUINT32_TO_BE(x) htobe32(x)
		#else
			uint32_t GUINT32_TO_BE(uint32_t x) {
				union { uint32_t u32; uint8_t v[4]; } ret;
				ret.v[0] = (uint8_t)(x >> 24);
				ret.v[1] = (uint8_t)(x >> 16);
				ret.v[2] = (uint8_t)(x >> 8);
				ret.v[3] = (uint8_t) x;
				return ret.u32;
			}
		#endif

		// if POSIX htobe64 is available use it
		#ifdef htobe64
		#define GUINT64_TO_BE(x) htobe64(x)
		#else
			uint64_t GUINT64_TO_BE(uint64_t x) {
				union { uint64_t u64; uint8_t v[8]; } ret;
				ret.v[0] = (uint8_t)(x >> 56);
				ret.v[1] = (uint8_t)(x >> 48);
				ret.v[2] = (uint8_t)(x >> 40);
				ret.v[3] = (uint8_t)(x >> 32);
				ret.v[4] = (uint8_t)(x >> 24);
				ret.v[5] = (uint8_t)(x >> 16);
				ret.v[6] = (uint8_t)(x >> 8);
				ret.v[7] = (uint8_t) x;
				return ret.u64;
			}
		#endif
	#else
		// system byte order is equal to network byte order
		// generate dummy macros
		#define GUINT16_TO_BE(x) (x)
		#define GUINT32_TO_BE(x) (x)
		#define GUINT64_TO_BE(x) (x)
	#endif
#endif

#endif /* IBRDTN_ENDIAN_C_ */
