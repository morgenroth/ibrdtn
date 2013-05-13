/*
 * Utils.h
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

#ifndef UTILS_H_
#define UTILS_H_

#include "ibrdtn/data/Bundle.h"
#include <ibrcommon/data/BLOB.h>
#include <list>

namespace dtn
{
	namespace utils
	{
		class Utils
		{
		public:
			static void rtrim(std::string &str);
			static void ltrim(std::string &str);
			static void trim(std::string &str);

			static std::vector<std::string> tokenize(const std::string &token, const std::string &data, const std::string::size_type max = std::string::npos);
			static double distance(double lat1, double lon1, double lat2, double lon2);

			static void encapsule(dtn::data::Bundle &capsule, const std::list<dtn::data::Bundle> &bundles);
			static void decapsule(const dtn::data::Bundle &capsule, std::list<dtn::data::Bundle> &bundles);

			static std::string toString(const dtn::data::Length &value);

		private:
			static void encapsule(ibrcommon::BLOB::Reference &ref, const std::list<dtn::data::Bundle> &bundles);
			static double toRad(double value);
			static const double pi;
		};
	}
}

#endif /*UTILS_H_*/
