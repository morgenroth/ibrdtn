/*
 * Random.cpp
 *
 *  Created on: 15.06.2011
 *      Author: morgenro
 */

#include "Random.h"
#include <time.h>
#include <stdlib.h>

namespace dtn
{
	namespace utils
	{
		Random::Random()
		{
			// initialize a random seed
			srand(time(0));
		}

		Random::~Random()
		{
		}

		const std::string Random::gen_chars(size_t size) const
		{
			static const char text[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
			char dst[size];
			int i, len = size - 1;
			for ( i = 0; i < len; ++i )
			{
				dst[i] = text[rand() % (sizeof text - 1)];
			}
			dst[i] = '\0';
			return dst;
		};
	}
}
