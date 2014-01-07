/*
 * Endianess.cpp
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

#include "ibrdtn/config.h"
#include "ibrdtn/data/Endianess.h"

namespace dtn
{
	namespace data
	{
		uint16_t bswap16(uint16_t x) {
			union { uint16_t u16; uint8_t v[2]; } ret;
			ret.v[0] = (uint8_t)(x >> 8);
			ret.v[1] = (uint8_t) x;
			return ret.u16;
		}

		uint32_t bswap32(uint32_t x) {
			union { uint32_t u32; uint8_t v[4]; } ret;
			ret.v[0] = (uint8_t)(x >> 24);
			ret.v[1] = (uint8_t)(x >> 16);
			ret.v[2] = (uint8_t)(x >> 8);
			ret.v[3] = (uint8_t) x;
			return ret.u32;
		}

		uint64_t bswap64(uint64_t x) {
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
	}
}
