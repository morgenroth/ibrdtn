/*
 * AcknowledgementSet.cpp
 *
 *  Created on: 11.01.2013
 *      Author: morgenro
 */

#include "routing/prophet/AcknowledgementSet.h"
#include <ibrdtn/utils/Clock.h>

namespace dtn
{
	namespace routing
	{
		const size_t AcknowledgementSet::identifier = NodeHandshakeItem::PROPHET_ACKNOWLEDGEMENT_SET;

		AcknowledgementSet::AcknowledgementSet()
		{
		}

		AcknowledgementSet::AcknowledgementSet(const AcknowledgementSet &other)
		 : ibrcommon::Mutex(), dtn::data::BundleList((const dtn::data::BundleList&)other)
		{
		}

		void AcknowledgementSet::merge(const AcknowledgementSet &other)
		{
			for(AcknowledgementSet::const_iterator it = other.begin(); it != other.end(); ++it)
			{
				const dtn::data::MetaBundle &ack = (*it);
				if (!dtn::utils::Clock::isExpired(ack.expiretime)) {
					add(ack);
				}
			}
		}

		bool AcknowledgementSet::has(const dtn::data::BundleID &bundle) const
		{
			return find(bundle) != end();
		}

		size_t AcknowledgementSet::getIdentifier() const
		{
			return identifier;
		}

		size_t AcknowledgementSet::getLength() const
		{
			std::stringstream ss;
			serialize(ss);
			return ss.str().length();
		}

		std::ostream& AcknowledgementSet::serialize(std::ostream& stream) const
		{
			stream << (*this);
			return stream;
		}

		std::istream& AcknowledgementSet::deserialize(std::istream& stream)
		{
			stream >> (*this);
			return stream;
		}

		std::ostream& operator<<(std::ostream& stream, const AcknowledgementSet& ack_set)
		{
			stream << dtn::data::SDNV(ack_set.size());
			for (dtn::data::BundleList::const_iterator it = ack_set.begin(); it != ack_set.end(); ++it)
			{
				const dtn::data::MetaBundle &ack = (*it);
				stream << (const dtn::data::BundleID&)ack;
				stream << dtn::data::SDNV(ack.expiretime);
			}

			return stream;
		}

		std::istream& operator>>(std::istream &stream, AcknowledgementSet &ack_set)
		{
			// clear the ack set first
			ack_set.clear();

			dtn::data::SDNV size;
			stream >> size;

			for(size_t i = 0; i < size.getValue(); i++)
			{
				dtn::data::MetaBundle ack;
				dtn::data::SDNV expire_time;
				stream >> (dtn::data::BundleID&)ack;
				stream >> expire_time;
				ack.expiretime = expire_time.getValue();

				ack_set.add(ack);
			}
			return stream;
		}
	} /* namespace routing */
} /* namespace dtn */
