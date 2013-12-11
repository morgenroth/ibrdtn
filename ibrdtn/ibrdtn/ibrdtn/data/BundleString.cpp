/*
 * BundleString.cpp
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

#include "ibrdtn/config.h"
#include "ibrdtn/data/BundleString.h"
#include "ibrdtn/data/Number.h"
#include <vector>

namespace dtn
{
	namespace data
	{
		BundleString::BundleString(const std::string &value)
		 : std::string(value)
		{
		}

		BundleString::BundleString()
		{
		}

		BundleString::~BundleString()
		{
		}

		Length BundleString::getLength() const
		{
			const dtn::data::Number str_len(length());
			return str_len.getLength() + length();
		}

		std::ostream &operator<<(std::ostream &stream, const BundleString &bstring)
		{
			const std::string &data = (std::string)bstring;
			const dtn::data::Number str_len(data.length());
			stream << str_len << data;
			return stream;
		}

		std::istream &operator>>(std::istream &stream, BundleString &bstring)
		{
			dtn::data::Number length;
			stream >> length;
			const std::streamsize data_len = length.get<std::streamsize>();

			// clear the content
			((std::string&)bstring) = "";
			
			if (data_len > 0)
			{
				std::vector<char> data(data_len);
				stream.read(&data[0], data.size());
				bstring.assign(&data[0], data.size());
			}

			return stream;
		}
	}
}
