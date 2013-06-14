/*
 * Random.cpp
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

#include "Random.h"
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <sys/time.h>

namespace dtn
{
	namespace utils
	{
		Random::Random()
		{
			static bool initialized = false;

			if (!initialized) {
				struct timeval time;
				::gettimeofday(&time, NULL);

				// initialize a random seed only once
				srand(static_cast<unsigned int>((time.tv_sec * 100) + (time.tv_usec / 100)));
				initialized = true;
			}
		}

		Random::~Random()
		{
		}

		const std::string Random::gen_chars(const dtn::data::Length &size) const
		{
			static const char text[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
			std::vector<char> dst(size);
			const dtn::data::Length len = size - 1;
			for ( dtn::data::Length i = 0; i <= len; ++i )
			{
				dst[i] = text[rand() % (sizeof text - 1)];
			}
			return std::string(dst.begin(), dst.end());
		}
	}
}
