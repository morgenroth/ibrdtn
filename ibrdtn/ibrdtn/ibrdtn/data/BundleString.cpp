/*
 * BundleString.cpp
 *
 *  Created on: 11.09.2009
 *      Author: morgenro
 */

#include "ibrdtn/config.h"
#include "ibrdtn/data/BundleString.h"
#include "ibrdtn/data/SDNV.h"

namespace dtn
{
	namespace data
	{
		BundleString::BundleString(std::string value)
		 : std::string(value)
		{
		}

		BundleString::BundleString()
		{
		}

		BundleString::~BundleString()
		{
		}

		size_t BundleString::getLength() const
		{
			dtn::data::SDNV sdnv(length());
			return sdnv.getLength() + length();
		}

		std::ostream &operator<<(std::ostream &stream, const BundleString &bstring)
		{
			std::string data = (std::string)bstring;
			dtn::data::SDNV sdnv(data.length());
			stream << sdnv << data;
			return stream;
		}

		std::istream &operator>>(std::istream &stream, BundleString &bstring)
		{
			dtn::data::SDNV length;
			stream >> length;
			std::streamsize data_len = length.getValue();

			// clear the content
			((std::string&)bstring) = "";
			
			if (data_len > 0)
			{
				char data[data_len];
				stream.read(data, data_len);
				bstring.assign(data, data_len);
			}

			return stream;
		}
	}
}
