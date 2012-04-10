/*
 * BundleID.cpp
 *
 *  Created on: 01.09.2009
 *      Author: morgenro
 */

#include "ibrdtn/config.h"
#include "ibrdtn/data/BundleID.h"
#include "ibrdtn/data/SDNV.h"
#include "ibrdtn/data/BundleString.h"

namespace dtn
{
	namespace data
	{
		BundleID::BundleID(const dtn::data::EID s, const size_t t, const size_t sq, const bool f, const size_t o)
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
