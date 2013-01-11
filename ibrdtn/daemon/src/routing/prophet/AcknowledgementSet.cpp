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
			: ibrcommon::Mutex(), _ackSet(other._ackSet)
		{
		}

		void AcknowledgementSet::insert(const Acknowledgement& ack)
		{
			_ackSet.insert(ack);
		}

		void AcknowledgementSet::clear()
		{
			_ackSet.clear();
		}

		void AcknowledgementSet::purge()
		{
			std::set<Acknowledgement>::iterator it;
			for(it = _ackSet.begin(); it != _ackSet.end();)
			{
				const Acknowledgement &ack = (*it);
				if(dtn::utils::Clock::isExpired(ack.expire_time))
					_ackSet.erase(it++);
				else
					++it;
			}
		}

		void AcknowledgementSet::merge(const AcknowledgementSet &other)
		{
			std::set<Acknowledgement>::iterator it;
			for(it = other._ackSet.begin(); it != other._ackSet.end(); ++it)
			{
				const Acknowledgement &ack = (*it);
				if (!dtn::utils::Clock::isExpired(ack.expire_time)) {
					_ackSet.insert(ack);
				}
			}
		}

		bool AcknowledgementSet::has(const dtn::data::BundleID &bundle) const
		{
			return _ackSet.find(Acknowledgement(bundle, 0)) != _ackSet.end();
		}

		size_t AcknowledgementSet::getIdentifier() const
		{
			return identifier;
		}

		size_t AcknowledgementSet::size() const
		{
			return _ackSet.size();
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

		const std::set<Acknowledgement>& AcknowledgementSet::operator*() const
		{
			return _ackSet;
		}

		std::ostream& operator<<(std::ostream& stream, const AcknowledgementSet& ack_set)
		{
			stream << dtn::data::SDNV(ack_set.size());
			for (std::set<Acknowledgement>::const_iterator it = ack_set._ackSet.begin(); it != ack_set._ackSet.end(); ++it)
			{
				const Acknowledgement &ack = (*it);
				stream << ack;
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
				Acknowledgement ack;
				stream >> ack;
				ack_set.insert(ack);
			}
			return stream;
		}
	} /* namespace routing */
} /* namespace dtn */
