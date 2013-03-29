/*
 * BundleID.cpp
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
#include "ibrdtn/data/BundleID.h"
#include "ibrdtn/data/SDNV.h"
#include "ibrdtn/data/BundleString.h"

namespace dtn
{
	namespace data
	{
		BundleID::BundleID(const dtn::data::EID s, const uint64_t t, const uint64_t sq, const bool f, const uint64_t o)
		: source(s), timestamp(t), sequencenumber(sq), fragment(f), offset(o)
		{
		}

		BundleID::BundleID(const dtn::data::Bundle &b)
		: source(b._source), timestamp(b._timestamp), sequencenumber(b._sequencenumber),
		fragment(b.get(dtn::data::Bundle::FRAGMENT)), offset(b._fragmentoffset)
		{
		}

		BundleID::~BundleID()
		{
		}

		bool BundleID::operator<(const BundleID& other) const
		{
			if (source < other.source) return true;
			if (source != other.source) return false;

			if (timestamp < other.timestamp) return true;
			if (timestamp != other.timestamp) return false;

			if (sequencenumber < other.sequencenumber) return true;
			if (sequencenumber != other.sequencenumber) return false;

			if (other.fragment)
			{
				if (!fragment) return true;
				return (offset < other.offset);
			}

			return false;
		}

		bool BundleID::operator>(const BundleID& other) const
		{
			return !(((*this) < other) || ((*this) == other));
		}

		bool BundleID::operator!=(const BundleID& other) const
		{
			return !((*this) == other);
		}

		bool BundleID::operator==(const BundleID& other) const
		{
			if (other.timestamp != timestamp) return false;
			if (other.sequencenumber != sequencenumber) return false;
			if (other.source != source) return false;
			if (other.fragment != fragment) return false;

			if (fragment)
			{
				if (other.offset != offset) return false;
			}

			return true;
		}

		size_t BundleID::getTimestamp() const
		{
			return timestamp;
		}

		string BundleID::toString() const
		{
			stringstream ss;
			ss << "[" << timestamp << "." << sequencenumber;

			if (fragment)
			{
				ss << "." << offset;
			}

			ss << "] " << source.getString();

			return ss.str();
		}

		std::ostream &operator<<(std::ostream &stream, const BundleID &obj)
		{
			dtn::data::SDNV timestamp(obj.timestamp);
			dtn::data::SDNV sequencenumber(obj.sequencenumber);
			dtn::data::SDNV offset(obj.offset);
			dtn::data::BundleString source(obj.source.getString());

			stream << timestamp << sequencenumber << offset << source;

			return stream;
		}

		std::istream &operator>>(std::istream &stream, BundleID &obj)
		{
			dtn::data::SDNV timestamp;
			dtn::data::SDNV sequencenumber;
			dtn::data::SDNV offset;
			dtn::data::BundleString source;

			stream >> timestamp >> sequencenumber >> offset >> source;

			obj.timestamp = timestamp.getValue();
			obj.sequencenumber = sequencenumber.getValue();
			obj.offset = offset.getValue();
			obj.source = dtn::data::EID(source);

			return stream;
		}
	}
}
