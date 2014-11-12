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
		BundleTransfer::BundleTransfer(const dtn::data::EID &neighbor, const dtn::data::MetaBundle &bundle, dtn::core::Node::Protocol p)
		 : _slot(new Slot(neighbor, bundle, p))
		{
		}

		BundleTransfer::~BundleTransfer()
		{
		}

		BundleTransfer::Slot::Slot(const dtn::data::EID &n, const dtn::data::MetaBundle &b, dtn::core::Node::Protocol p)
		 : neighbor(n), bundle(b), protocol(p), _completed(false), _aborted(false), _abort_reason(TransferAbortedEvent::REASON_UNDEFINED)
		{
		}

		BundleTransfer::Slot::~Slot()
		{
			if (_aborted) {
				// fire TransferAbortedEvent
				dtn::net::TransferAbortedEvent::raise(neighbor, bundle, _abort_reason);
			} else if (_completed) {
				dtn::net::TransferCompletedEvent::raise(neighbor, bundle);
				dtn::core::BundleEvent::raise(bundle, dtn::core::BUNDLE_FORWARDED);
			} else {
				dtn::routing::RequeueBundleEvent::raise(neighbor, bundle, protocol);
			}
		}

		const dtn::data::EID& BundleTransfer::getNeighbor() const
		{
			return _slot->neighbor;
		}

		const dtn::data::MetaBundle& BundleTransfer::getBundle() const
		{
			return _slot->bundle;
		}

		dtn::core::Node::Protocol BundleTransfer::getProtocol() const{
			return _slot->protocol;
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
