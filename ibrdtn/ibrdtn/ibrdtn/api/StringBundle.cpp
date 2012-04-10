/*
 * StringBundle.cpp
 *
 *  Created on: 24.07.2009
 *      Author: morgenro
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
