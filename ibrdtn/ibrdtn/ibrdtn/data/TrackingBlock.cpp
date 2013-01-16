/*
 * TrackingBlock.cpp
 *
 *  Created on: 15.01.2013
 *      Author: morgenro
 */

#include "ibrdtn/data/TrackingBlock.h"
#include "ibrdtn/data/BundleString.h"

namespace dtn
{
	namespace data
	{
		dtn::data::Block* TrackingBlock::Factory::create()
		{
			return new TrackingBlock();
		}

		TrackingBlock::TrackingBlock()
		 : dtn::data::Block(TrackingBlock::BLOCK_TYPE)
		{
		}

		TrackingBlock::~TrackingBlock()
		{
		}

		size_t TrackingBlock::getLength() const
		{
			size_t ret = 0;

			// number of elements
			dtn::data::SDNV count(_entries.size());
			ret += count.getLength();

			for (tracking_list::const_iterator iter = _entries.begin(); iter != _entries.end(); iter++)
			{
				const TrackingEntry &entry = (*iter);
				ret += entry.getLength();
			}

			return ret;
		}

		std::ostream& TrackingBlock::serialize(std::ostream &stream, size_t&) const
		{
			// number of elements
			dtn::data::SDNV count(_entries.size());
			stream << count;

			for (tracking_list::const_iterator iter = _entries.begin(); iter != _entries.end(); iter++)
			{
				const TrackingEntry &entry = (*iter);
				stream << entry;
			}

			return stream;
		}

		std::istream& TrackingBlock::deserialize(std::istream &stream, const size_t)
		{
			// number of elements
			dtn::data::SDNV count;

			stream >> count;

			for (size_t i = 0; i < count.getValue(); i++)
			{
				TrackingEntry entry;
				stream >> entry;
				_entries.push_back(entry);
			}

			return stream;
		}

		size_t TrackingBlock::getLength_strict() const
		{
			return getLength();
		}

		std::ostream& TrackingBlock::serialize_strict(std::ostream &stream, size_t &length) const
		{
			return serialize(stream, length);
		}

		const TrackingBlock::tracking_list& TrackingBlock::getTrack() const
		{
			return _entries;
		}

		void TrackingBlock::append(const dtn::data::EID &eid)
		{
			TrackingEntry entry(eid);

			// include timestamp
			entry.setFlag(TrackingEntry::TIMESTAMP_PRESENT, true);

			// use default timestamp
			//entry.timestamp.set();

			_entries.push_back(entry);
		}

		TrackingBlock::TrackingEntry::TrackingEntry()
		 : flags(0)
		{
		}

		TrackingBlock::TrackingEntry::TrackingEntry(const dtn::data::EID &eid)
		 : flags(0), endpoint(eid)
		{
		}

		TrackingBlock::TrackingEntry::~TrackingEntry()
		{
		}

		bool TrackingBlock::TrackingEntry::getFlag(TrackingBlock::TrackingEntry::Flags f) const
		{
			return (flags & f);
		}

		void TrackingBlock::TrackingEntry::setFlag(TrackingBlock::TrackingEntry::Flags f, bool value)
		{
			if (value)
			{
				flags |= f;
			}
			else
			{
				flags &= ~(f);
			}
		}

		size_t TrackingBlock::TrackingEntry::getLength() const
		{
			size_t ret = dtn::data::SDNV(flags).getLength();

			if (getFlag(TrackingEntry::TIMESTAMP_PRESENT)) {
				ret += timestamp.getLength();
			}

			ret += BundleString(endpoint.getString()).getLength();

			return ret;
		}

		std::ostream& operator<<(std::ostream &stream, const TrackingBlock::TrackingEntry &entry)
		{
			stream << dtn::data::SDNV(entry.flags);

			if (entry.getFlag(TrackingBlock::TrackingEntry::TIMESTAMP_PRESENT)) {
				stream << entry.timestamp;
			}

			dtn::data::BundleString endpoint(entry.endpoint.getString());
			stream << endpoint;
			return stream;
		}

		std::istream& operator>>(std::istream &stream, TrackingBlock::TrackingEntry &entry)
		{
			dtn::data::SDNV flags;
			stream >> flags;
			entry.flags = flags.getValue();

			if (entry.getFlag(TrackingBlock::TrackingEntry::TIMESTAMP_PRESENT)) {
				stream >> entry.timestamp;
			}

			BundleString endpoint;
			stream >> endpoint;
			entry.endpoint = dtn::data::EID((std::string&)endpoint);
			return stream;
		}
	} /* namespace data */
} /* namespace dtn */
