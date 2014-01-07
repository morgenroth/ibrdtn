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
#include <glib.h>
#elif HAVE_ENDIAN_H
#include <endian.h>
#define GUINT16_TO_BE(x) htobe16(x)
#define GUINT32_TO_BE(x) htobe32(x)
#define GUINT64_TO_BE(x) htobe64(x)
#else
#ifdef HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#elif __WIN32__
#include <winsock2.h>
#define BYTE_ORDER __BYTE_ORDER
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
	uint16_t GUINT16_TO_BE(uint16_t x) {
		union { uint16_t u16; uint8_t v[2]; } ret;
		ret.v[0] = (uint8_t)(x >> 8);
		ret.v[1] = (uint8_t) x;
		return ret.u16;
	}

	uint32_t GUINT32_TO_BE(uint32_t x) {
		union { uint32_t u32; uint8_t v[4]; } ret;
		ret.v[0] = (uint8_t)(x >> 24);
		ret.v[1] = (uint8_t)(x >> 16);
		ret.v[2] = (uint8_t)(x >> 8);
		ret.v[3] = (uint8_t) x;
		return ret.u32;
	}

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
#else
	#define GUINT16_TO_BE(x) (x)
	#define GUINT32_TO_BE(x) (x)
	#define GUINT64_TO_BE(x) (x)
#endif
#endif

#endif /* IBRDTN_ENDIAN_C_ */
