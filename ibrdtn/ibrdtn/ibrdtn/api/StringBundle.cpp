/*
 * StringBundle.cpp
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
#include "ibrdtn/api/StringBundle.h"

namespace dtn
{
	namespace api
	{
		StringBundle::StringBundle(const dtn::data::EID &destination)
		 : Bundle(destination), _ref(ibrcommon::BLOB::create())
		{
			_b.push_back(_ref);
		}

		StringBundle::~StringBundle()
		{ }

		void StringBundle::append(const std::string &data)
		{
			ibrcommon::BLOB::iostream stream = _ref.iostream();
			(*stream).seekp(0, ios::end);
			(*stream) << data;
		}
	}
}
