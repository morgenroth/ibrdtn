/*
 * MetaBundle.cpp
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

#ifndef METABUNDLE_CPP_
#define METABUNDLE_CPP_

#include "ibrdtn/data/MetaBundle.h"
#include "ibrdtn/utils/Clock.h"
#include "ibrdtn/data/ScopeControlHopLimitBlock.h"
#include "ibrdtn/data/SchedulingBlock.h"
#include <limits>

namespace dtn
{
	namespace data
	{
		MetaBundle::MetaBundle()
		 : BundleID(), received(), lifetime(0), destination(), reportto(),
		   custodian(), appdatalength(0), procflags(0), expiretime(0), hopcount(std::numeric_limits<std::size_t>::max()), payloadlength(0), net_priority(0)
		{
		}

		MetaBundle::MetaBundle(const dtn::data::BundleID &id)
		 : BundleID(id), received(), lifetime(0), destination(), reportto(),
		   custodian(), appdatalength(0), procflags(0), expiretime(0), hopcount(std::numeric_limits<std::size_t>::max()), payloadlength(0), net_priority(0)
		{
		}

		MetaBundle::MetaBundle(const dtn::data::Bundle &b)
		 : BundleID(b), lifetime(b.lifetime), destination(b._destination), reportto(b._reportto),
		   custodian(b._custodian), appdatalength(b.appdatalength), procflags(b.procflags), expiretime(0), hopcount(std::numeric_limits<std::size_t>::max()), payloadlength(0), net_priority(0)
		{
			expiretime = dtn::utils::Clock::getExpireTime(b);

			/**
			 * read the hop limit
			 */
			try {
				const dtn::data::ScopeControlHopLimitBlock &schl = b.find<const dtn::data::ScopeControlHopLimitBlock>();
				hopcount = schl.getHopsToLive();
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };

			/**
			 * read the scheduling block
			 */
			try {
				const dtn::data::SchedulingBlock &sblock = b.find<const dtn::data::SchedulingBlock>();
				net_priority = sblock.getPriority();
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };

			/**
			 * read the payload length
			 */
			try {
				const dtn::data::PayloadBlock &pblock = b.find<const dtn::data::PayloadBlock>();
				payloadlength = pblock.getLength();
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };
		}

		MetaBundle::~MetaBundle()
		{}

		int MetaBundle::getPriority() const
		{
			// read priority
			if (procflags & dtn::data::PrimaryBlock::PRIORITY_BIT1)
			{
				return 0;
			}

			if (procflags & dtn::data::PrimaryBlock::PRIORITY_BIT2)
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
