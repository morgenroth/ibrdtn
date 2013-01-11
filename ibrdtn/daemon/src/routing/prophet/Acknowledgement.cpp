/*
 * Acknowledgement.cpp
 *
 *  Created on: 11.01.2013
 *      Author: morgenro
 */

#include "routing/prophet/Acknowledgement.h"

namespace dtn
{
	namespace routing
	{
		Acknowledgement::Acknowledgement()
		 : expire_time(0)
		{
		}

		Acknowledgement::Acknowledgement(const dtn::data::BundleID &bundleID, size_t expire_time)
			: bundleID(bundleID), expire_time(expire_time)
		{
		}

		Acknowledgement::~Acknowledgement()
		{
		}

		bool Acknowledgement::operator<(const Acknowledgement &other) const
		{
			return (bundleID < other.bundleID);
		}

		std::ostream& operator<<(std::ostream &stream, const Acknowledgement &ack) {
			stream << ack.bundleID;
			stream << dtn::data::SDNV(ack.expire_time);
			return stream;
		}

		std::istream& operator>>(std::istream &stream, Acknowledgement &ack) {
			dtn::data::SDNV expire_time;
			stream >> ack.bundleID;
			stream >> expire_time;
			ack.expire_time = expire_time.getValue();
			return stream;
		}
	} /* namespace routing */
} /* namespace dtn */
