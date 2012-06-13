/*
 * BundleString.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef BUNDLESTRING_H_
#define BUNDLESTRING_H_

#include <string>

namespace dtn
{
	namespace data
	{
		class BundleString : public std::string
		{
		public:
			BundleString(std::string value);
			BundleString();
			virtual ~BundleString();
			
			/**
			 * This method returns the length of the full encoded bundle string.
			 * @return The length of the full encoded bundle string
			 */
			size_t getLength() const;

		private:
			friend std::ostream &operator<<(std::ostream &stream, const BundleString &bstring);
			friend std::istream &operator>>(std::istream &stream, BundleString &bstring);
		};
	}
}

#endif /* BUNDLESTRING_H_ */
