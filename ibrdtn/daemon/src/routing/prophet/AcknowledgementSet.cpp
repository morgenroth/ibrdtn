/*
 * AcknowledgementSet.cpp
 *
 *  Created on: 11.01.2013
 *      Author: morgenro
 */

#include "routing/prophet/AcknowledgementSet.h"
#include "core/BundleCore.h"
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
		 : ibrcommon::Mutex(), _bundles(other._bundles)
		{
		}

		void AcknowledgementSet::add(const dtn::data::MetaBundle &bundle) throw ()
		{
			try {
				// check if the bundle is valid
				dtn::core::BundleCore::getInstance().validate(bundle);

				// add the bundle to the set
				_bundles.add(bundle);
			} catch (dtn::data::Validator::RejectedException &ex) {
				// bundle rejected
			}
		}

		void AcknowledgementSet::expire(size_t timestamp) throw ()
		{
			_bundles.expire(timestamp);
		}

		void AcknowledgementSet::merge(const AcknowledgementSet &other) throw ()
		{
			for(dtn::data::BundleList::const_iterator it = other._bundles.begin(); it != other._bundles.end(); ++it)
			{
				const dtn::data::MetaBundle &ack = (*it);
				this->add(ack);
			}
		}

		bool AcknowledgementSet::has(const dtn::data::BundleID &id) const throw ()
		{
			dtn::data::BundleList::const_iterator iter = _bundles.find(id);
			return !(iter == _bundles.end());
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
			stream << dtn::data::SDNV(ack_set._bundles.size());
			for (dtn::data::BundleList::const_iterator it = ack_set._bundles.begin(); it != ack_set._bundles.end(); ++it)
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
			ack_set._bundles.clear();

			dtn::data::SDNV size;
			stream >> size;

			for(size_t i = 0; i < size.getValue(); ++i)
			{
				dtn::data::MetaBundle ack;
				dtn::data::SDNV expire_time;
				stream >> (dtn::data::BundleID&)ack;
				stream >> expire_time;
				ack.expiretime = expire_time.getValue();
				ack.lifetime = dtn::utils::Clock::getLifetime(ack, ack.expiretime);

				ack_set.add(ack);
			}
			return stream;
		}
	} /* namespace routing */
} /* namespace dtn */
