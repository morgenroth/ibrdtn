/*
 * CustodySignalBlock.cpp
 *
 *  Created on: 05.06.2009
 *      Author: morgenro
 */

#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/CustodySignalBlock.h"
#include "ibrdtn/data/BundleString.h"
#include <stdlib.h>
#include <sstream>

namespace dtn
{
	namespace data
	{
		CustodySignalBlock::CustodySignalBlock()
		 : Block(dtn::data::PayloadBlock::BLOCK_TYPE), _admfield(32), _custody_accepted(false), _reason(NO_ADDITIONAL_INFORMATION), _fragment_offset(0),
		 _fragment_length(0), _timeofsignal(), _bundle_timestamp(0), _bundle_sequence(0)
		{
		}

		CustodySignalBlock::~CustodySignalBlock()
		{
		}

		size_t CustodySignalBlock::getLength() const
		{
			// determine the block size
			size_t len = 0;
			len += sizeof(_admfield);
			len += sizeof(char); // status

			if ( _admfield & 0x01 )
			{
				len += _fragment_offset.getLength();
				len += _fragment_length.getLength();
			}

			len += _timeofsignal.getLength();
			len += _bundle_timestamp.getLength();
			len += _bundle_sequence.getLength();

			BundleString sourceid(_source.getString());
			len += sourceid.getLength();

			return len;
		}

		std::istream& CustodySignalBlock::deserialize(std::istream &stream, const size_t length)
		{
			stream >> _admfield;

			char status; stream >> status;

			// decode custody acceptance
			_custody_accepted = (status & 0x01);

			// decode reason flag
			_reason = REASON_CODE(status >> 1);

			if ( _admfield & 0x01 )
			{
				stream >> _fragment_offset;
				stream >> _fragment_length;
			}

			stream >> _timeofsignal;
			stream >> _bundle_timestamp;
			stream >> _bundle_sequence;

			BundleString source;
			stream >> source;
			_source = EID(source);

			// unset block not processed bit
			set(dtn::data::Block::FORWARDED_WITHOUT_PROCESSED, false);

			return stream;
		}

		std::ostream& CustodySignalBlock::serialize(std::ostream &stream, size_t &length) const
		{
			// write the content
			stream << _admfield;

			// encode reason flag
			char status = (_reason << 1);

			// encode custody acceptance
			if (_custody_accepted) status |= 0x01;

			// write the status byte
			stream << status;

			if ( _admfield & 0x01 )
			{
				stream << _fragment_offset;
				stream << _fragment_length;
			}

			BundleString sourceid(_source.getString());

			stream << _timeofsignal
			   << _bundle_timestamp
			   << _bundle_sequence
			   << sourceid;

			return stream;
		}

		void CustodySignalBlock::setMatch(const dtn::data::MetaBundle& other)
		{
			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				_fragment_offset = other.offset;
				_fragment_length = other.appdatalength;

				if (!(_admfield & 1)) _admfield += 1;
			}

			_bundle_timestamp = other.timestamp;
			_bundle_sequence = other.sequencenumber;
			_source = other.source;
		}

		void CustodySignalBlock::setMatch(const dtn::data::Bundle& other)
		{
			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				_fragment_offset = other._fragmentoffset;
				_fragment_length = other._appdatalength;

				if (!(_admfield & 1)) _admfield += 1;
			}

			_bundle_timestamp = other._timestamp;
			_bundle_sequence = other._sequencenumber;
			_source = other._source;
		}

		bool CustodySignalBlock::match(const Bundle& other) const
		{
			if (_bundle_timestamp != other._timestamp) return false;
			if (_bundle_sequence != other._sequencenumber) return false;
			if (_source != other._source) return false;

			// set bundle parameter
			if (other.get(Bundle::FRAGMENT))
			{
				if (!(_admfield & 1)) return false;
				if (_fragment_offset != other._fragmentoffset) return false;
				if (_fragment_length != other._appdatalength) return false;
			}

			return true;
		}

	}
}
