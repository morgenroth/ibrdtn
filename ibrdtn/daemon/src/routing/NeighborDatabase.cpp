/*
 * NeighborDatabase.cpp
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

#include "routing/NeighborDatabase.h"
#include "core/BundleCore.h"
#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/Logger.h>
#include <limits>

namespace dtn
{
	namespace routing
	{
		NeighborDatabase::NeighborEntry::NeighborEntry()
		 : eid(), _filter(), _filter_expire(0), _filter_state(FILTER_EXPIRED, FILTER_FINAL)
		{};

		NeighborDatabase::NeighborEntry::NeighborEntry(const dtn::data::EID &e)
		 : eid(e), _filter(), _filter_expire(0), _filter_state(FILTER_EXPIRED, FILTER_FINAL)
		{ }

		NeighborDatabase::NeighborEntry::~NeighborEntry()
		{ }

		void NeighborDatabase::NeighborEntry::update(const ibrcommon::BloomFilter &bf, const size_t lifetime)
		{
			ibrcommon::ThreadsafeState<FILTER_REQUEST_STATE>::Locked l = _filter_state.lock();
			_filter = bf;

			if (lifetime == 0)
			{
				_filter_expire = std::numeric_limits<std::size_t>::max();
			}
			else
			{
				_filter_expire = dtn::utils::Clock::getExpireTime(lifetime);
			}

			l = FILTER_AVAILABLE;
		}

		void NeighborDatabase::NeighborEntry::reset()
		{
			ibrcommon::ThreadsafeState<FILTER_REQUEST_STATE>::Locked l = _filter_state.lock();

			l = FILTER_EXPIRED;

			// do not expire again in the next 60 seconds
			_filter_expire = dtn::utils::Clock::getTime() + 60;
		}

		void NeighborDatabase::NeighborEntry::add(const dtn::data::MetaBundle &bundle)
		{
			_summary.add(bundle);
		}

		bool NeighborDatabase::NeighborEntry::has(const dtn::data::BundleID &id, const bool require_bloomfilter) const
		{
			if (require_bloomfilter && (_filter_state != FILTER_AVAILABLE))
				throw BloomfilterNotAvailableException(eid);

			if (_filter_state == FILTER_AVAILABLE)
			{
				if (_filter.contains(id.toString()))
					return true;
			}

			if (_summary.contains(id))
				return true;

			return false;
		}

		void NeighborDatabase::NeighborEntry::expire(const size_t timestamp)
		{
			{
				ibrcommon::ThreadsafeState<FILTER_REQUEST_STATE>::Locked l = _filter_state.lock();

				if ((_filter_expire > 0) && (_filter_expire < timestamp))
				{
					IBRCOMMON_LOGGER_DEBUG(15) << "summary vector of " << eid.getString() << " is expired" << IBRCOMMON_LOGGER_ENDL;

					// set the filter state to expired once
					l = FILTER_EXPIRED;

					// do not expire again in the next 60 seconds
					_filter_expire = timestamp + 60;
				}
			}

			_summary.expire(timestamp);
		}

		void NeighborDatabase::expire(const size_t timestamp)
		{
			for (std::map<dtn::data::EID, NeighborDatabase::NeighborEntry* >::const_iterator iter = _entries.begin(); iter != _entries.end(); iter++)
			{
				(*iter).second->expire(timestamp);
			}
		}

		void NeighborDatabase::NeighborEntry::acquireFilterRequest() throw (NoMoreTransfersAvailable)
		{
			ibrcommon::ThreadsafeState<FILTER_REQUEST_STATE>::Locked l = _filter_state.lock();

			if (l != FILTER_EXPIRED)
				throw NoMoreTransfersAvailable();

			// set the state to zero
			l = FILTER_AWAITING;
		}

		void NeighborDatabase::NeighborEntry::acquireTransfer(const dtn::data::BundleID &id) throw (NoMoreTransfersAvailable, AlreadyInTransitException)
		{
			ibrcommon::MutexLock l(_transit_lock);

			// check if the bundle is already in transit
			if (_transit_bundles.find(id) != _transit_bundles.end()) throw AlreadyInTransitException();

			// check if enough resources available to transfer the bundle
			if (_transit_bundles.size() >= dtn::core::BundleCore::max_bundles_in_transit) throw NoMoreTransfersAvailable();

			// insert the bundle into the transit list
			_transit_bundles.insert(id);

			IBRCOMMON_LOGGER_DEBUG(20) << "acquire transfer of " << id.toString() << " (" << _transit_bundles.size() << " bundles in transit)" << IBRCOMMON_LOGGER_ENDL;
		}

		void NeighborDatabase::NeighborEntry::releaseTransfer(const dtn::data::BundleID &id)
		{
			ibrcommon::MutexLock l(_transit_lock);
			_transit_bundles.erase(id);

			IBRCOMMON_LOGGER_DEBUG(20) << "release transfer of " << id.toString() << " (" << _transit_bundles.size() << " bundles in transit)" << IBRCOMMON_LOGGER_ENDL;
		}

		NeighborDatabase::NeighborDatabase()
		{
		}

		NeighborDatabase::~NeighborDatabase()
		{
			std::set<dtn::data::EID> ret;

			for (std::map<dtn::data::EID, NeighborDatabase::NeighborEntry* >::const_iterator iter = _entries.begin(); iter != _entries.end(); iter++)
			{
				delete (*iter).second;
			}
		}

		NeighborDatabase::NeighborEntry& NeighborDatabase::create(const dtn::data::EID &eid)
		{
			if (_entries.find(eid) == _entries.end())
			{
				NeighborEntry *entry = new NeighborEntry(eid);
				_entries[eid] = entry;
			}

			return (*_entries[eid]);
		}

		NeighborDatabase::NeighborEntry& NeighborDatabase::get(const dtn::data::EID &eid) throw (NeighborNotAvailableException)
		{
			if (_entries.find(eid) == _entries.end())
			{
				throw NeighborNotAvailableException();
			}

			return (*_entries[eid]);
		}

		NeighborDatabase::NeighborEntry& NeighborDatabase::reset(const dtn::data::EID &eid)
		{
			NeighborDatabase::NeighborEntry &e = get(eid);
			e.reset();
			return e;
		}

		void NeighborDatabase::remove(const dtn::data::EID &eid)
		{
			_entries.erase(eid);
		}

		void NeighborDatabase::addBundle(const dtn::data::EID &neighbor, const dtn::data::MetaBundle &b)
		{
			try {
				NeighborDatabase::NeighborEntry &entry = get(neighbor);
				entry.add(b);
			} catch (const NeighborDatabase::NeighborNotAvailableException&) { };
		}
	}
}
