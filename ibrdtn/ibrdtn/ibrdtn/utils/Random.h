/*
 * Random.h
 *
 *  Created on: 15.06.2011
 *      Author: morgenro
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
			Random();
			virtual ~Random();

			const std::string gen_chars(size_t length) const;
		};
	}
}

#endif /* RANDOM_H_ */
