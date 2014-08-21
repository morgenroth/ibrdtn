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
		Random& Random::getInstance()
		{
			static Random instance;
			return instance;
		}

		Random::Random()
		{
			struct timeval time;
			::gettimeofday(&time, NULL);

			// initialize a random seed only once
			::srand(static_cast<unsigned int>((time.tv_sec * 100) + (time.tv_usec / 100)));
		}

		Random::~Random()
		{
		}

		int Random::gen_number()
		{
			return getInstance()._gen_number();
		}

		const std::string Random::gen_chars(const size_t &size)
		{
			return getInstance()._gen_chars(size);
		}

		int Random::_gen_number() const
		{
			return ::rand();
		}

		const std::string Random::_gen_chars(const size_t &size) const
		{
			static const char text[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
			std::vector<char> dst(size);
			const size_t len = size - 1;
			for ( size_t i = 0; i <= len; ++i )
			{
				dst[i] = text[::rand() % (sizeof text - 1)];
			}
			return std::string(dst.begin(), dst.end());
		}
	}
}
