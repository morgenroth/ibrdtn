/*
 * BundleTransfer.cpp
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#include "net/BundleTransfer.h"
#include "routing/RequeueBundleEvent.h"
#include "core/BundleEvent.h"
#include "net/TransferCompletedEvent.h"
#include <ibrcommon/thread/MutexLock.h>

namespace dtn
{
	namespace net
	{
		ibrcommon::Mutex BundleTransfer::Slot::_slot_lock;
		BundleTransfer::Slot::slot_map BundleTransfer::Slot::_slot_map;

		BundleTransfer::BundleTransfer(const dtn::data::EID &neighbor, const dtn::data::MetaBundle &bundle)
		 : _slot(new Slot(neighbor, bundle))
		{
		}

		BundleTransfer::~BundleTransfer()
		{
		}

		BundleTransfer::Slot::Slot(const dtn::data::EID &n, const dtn::data::MetaBundle &b)
		 : neighbor(n), bundle(b), _completed(false), _aborted(false), _abort_reason(TransferAbortedEvent::REASON_UNDEFINED)
		{
			ibrcommon::MutexLock l(_slot_lock);
			if (_slot_map.find(neighbor) == _slot_map.end())
			{
				_slot_map[neighbor] = 1;
			}
			else
			{
				++(_slot_map[neighbor]);
			}
		}

		BundleTransfer::Slot::~Slot()
		{
			ibrcommon::MutexLock l(_slot_lock);
			if (_slot_map[neighbor] == 1)
			{
				_slot_map.erase(neighbor);
			}
			else
			{
				--(_slot_map[neighbor]);
			}

			if (_aborted) {
				// fire TransferAbortedEvent
				dtn::net::TransferAbortedEvent::raise(neighbor, bundle, _abort_reason);
			} else if (_completed) {
				dtn::net::TransferCompletedEvent::raise(neighbor, bundle);
				dtn::core::BundleEvent::raise(bundle, dtn::core::BUNDLE_FORWARDED);
			} else {
				dtn::routing::RequeueBundleEvent::raise(neighbor, bundle);
			}
		}

		int BundleTransfer::count(const dtn::data::EID &neighbor)
		{
			return Slot::count(neighbor);
		}

		int BundleTransfer::Slot::count(const dtn::data::EID &neighbor)
		{
			ibrcommon::MutexLock l(_slot_lock);
			return _slot_map[neighbor];
		}

		const dtn::data::EID& BundleTransfer::getNeighbor() const
		{
			return _slot->neighbor;
		}

		const dtn::data::MetaBundle& BundleTransfer::getBundle() const
		{
			return _slot->bundle;
		}

		void BundleTransfer::Slot::abort(const TransferAbortedEvent::AbortReason reason)
		{
			_abort_reason = reason;
			_aborted = true;
		}

		void BundleTransfer::Slot::complete()
		{
			_completed = true;
		}

		void BundleTransfer::abort(const TransferAbortedEvent::AbortReason reason)
		{
			_slot->abort(reason);
		}

		void BundleTransfer::complete()
		{
			_slot->complete();
		}
	} /* namespace net */
} /* namespace dtn */
