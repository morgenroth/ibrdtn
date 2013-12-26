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
#include "ibrdtn/data/Number.h"
#include "ibrdtn/data/BundleString.h"

namespace dtn
{
	namespace data
	{
		BundleID::BundleID()
		 : source(), timestamp(0), sequencenumber(0), fragmentoffset(0), _fragment(false)
		{
		}

		BundleID::BundleID(const BundleID &id)
		 : source(id.source), timestamp(id.timestamp), sequencenumber(id.sequencenumber), fragmentoffset(id.fragmentoffset), _fragment(id.isFragment())
		{
		}

		BundleID::~BundleID()
		{
		}

		BundleID& BundleID::operator=(const BundleID &id)
		{
			source = id.source;
			timestamp = id.timestamp;
			sequencenumber = id.sequencenumber;
			fragmentoffset = id.fragmentoffset;
			setFragment(id._fragment);
			return (*this);
		}

		bool BundleID::operator<(const BundleID& other) const
		{
			if (source < other.source) return true;
			if (source != other.source) return false;

			if (timestamp < other.timestamp) return true;
			if (timestamp != other.timestamp) return false;

			if (sequencenumber < other.sequencenumber) return true;
			if (sequencenumber != other.sequencenumber) return false;

			if (other.isFragment())
			{
				if (!isFragment()) return true;
				return (fragmentoffset < other.fragmentoffset);
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
			if (other.isFragment() != isFragment()) return false;

			if (isFragment())
			{
				if (other.fragmentoffset != fragmentoffset) return false;
			}

			return true;
		}

		bool BundleID::isFragment() const
		{
			return _fragment;
		}

		void BundleID::setFragment(bool val)
		{
			_fragment = val;
		}

		void BundleID::addTo(ibrcommon::BloomFilter &bf) const
		{
			std::stringstream ss;
			ss << (*this);
			bf.insert(ss.str());
		}

		bool BundleID::isIn(const ibrcommon::BloomFilter &bf) const
		{
			std::stringstream ss;
			ss << (*this);
			return bf.contains(ss.str());
		}

		std::string BundleID::toString() const
		{
			stringstream ss;
			ss << "[" << timestamp.toString() << "." << sequencenumber.toString();

			if (isFragment())
			{
				ss << "." << fragmentoffset.toString();
			}

			ss << "] " << source.getString();

			return ss.str();
		}

		std::ostream &operator<<(std::ostream &stream, const BundleID &obj)
		{
			dtn::data::BundleString source(obj.source.getString());

			stream << obj.timestamp << obj.sequencenumber;

			if (obj.isFragment()) {
				stream.put(1);
				stream << obj.fragmentoffset;
			} else {
				stream.put(0);
			}

			stream << source;

			return stream;
		}

		std::istream &operator>>(std::istream &stream, BundleID &obj)
		{
			dtn::data::BundleString source;

			stream >> obj.timestamp >> obj.sequencenumber;

			uint8_t frag = 0;
			stream.get((char&)frag);

			if (frag == 1) {
				stream >> obj.fragmentoffset;
				obj.setFragment(true);
			} else {
				obj.setFragment(false);
			}

			stream >> source;
			obj.source = dtn::data::EID(source);

			return stream;
		}
	}
}
