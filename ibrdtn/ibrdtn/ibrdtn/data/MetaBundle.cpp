/*
 * MetaBundle.cpp
 *
 *  Created on: 16.02.2010
 *      Author: morgenro
 */

#ifndef METABUNDLE_CPP_
#define METABUNDLE_CPP_

#include "ibrdtn/data/MetaBundle.h"
#include "ibrdtn/utils/Clock.h"
#include "ibrdtn/data/ScopeControlHopLimitBlock.h"
#include <limits>

namespace dtn
{
	namespace data
	{
		MetaBundle::MetaBundle()
		 : BundleID(), received(), lifetime(0), destination(), reportto(),
		   custodian(), appdatalength(0), procflags(0), expiretime(0), hopcount(std::numeric_limits<std::size_t>::max()), payloadlength(0)
		{
		}

		MetaBundle::MetaBundle(const dtn::data::BundleID &id)
		 : BundleID(id), received(), lifetime(0), destination(), reportto(),
		   custodian(), appdatalength(0), procflags(0), expiretime(0), hopcount(std::numeric_limits<std::size_t>::max()), payloadlength(0)
		{
		}

		MetaBundle::MetaBundle(const dtn::data::Bundle &b)
		 : BundleID(b), lifetime(b._lifetime), destination(b._destination), reportto(b._reportto),
		   custodian(b._custodian), appdatalength(b._appdatalength), procflags(b._procflags), expiretime(0), hopcount(std::numeric_limits<std::size_t>::max()), payloadlength(0)
		{
			expiretime = dtn::utils::Clock::getExpireTime(b);

			/**
			 * read the hop limit
			 */
			try {
				const dtn::data::ScopeControlHopLimitBlock &schl = b.getBlock<const dtn::data::ScopeControlHopLimitBlock>();
				hopcount = schl.getHopsToLive();
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };

			/**
			 * read the payload length
			 */
			try {
				const dtn::data::PayloadBlock &pblock = b.getBlock<const dtn::data::PayloadBlock>();
				payloadlength = pblock.getLength();
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };
		}

		MetaBundle::~MetaBundle()
		{}

		int MetaBundle::getPriority() const
		{
			// read priority
			if (procflags & dtn::data::Bundle::PRIORITY_BIT1)
			{
				return 0;
			}

			if (procflags & dtn::data::Bundle::PRIORITY_BIT2)
			{
				return 1;
			}

			return -1;
		}

		bool MetaBundle::get(dtn::data::PrimaryBlock::FLAGS flag) const
		{
			return (procflags & flag);
		}
	}
}

#endif /* METABUNDLE_CPP_ */
