/*
 * Random.h
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

#ifndef RANDOM_H_
#define RANDOM_H_

#include <string>

namespace dtn
{
	namespace utils
	{
		class Random
		{
		public:
			virtual ~Random();

			/**
			 * Generates a 32-bit random number
			 */
			static int gen_number();

			/**
			 * Generate a random string using alphabetic chars
			 */
			static const std::string gen_chars(const size_t &length);

		private:
			Random();

			static Random& getInstance();

			// internal method to generate a number
			int _gen_number() const;

			// internal method to generate a random char
			const std::string _gen_chars(const size_t &length) const;
		};
	}
}

#endif /* RANDOM_H_ */
