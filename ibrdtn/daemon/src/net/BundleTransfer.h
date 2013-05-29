/*
 * BundleTransfer.h
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

#include "net/TransferAbortedEvent.h"
#include <ibrdtn/data/Number.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/MetaBundle.h>

#include <ibrcommon/thread/Mutex.h>
#include <map>

#ifndef BUNDLETRANSFER_H_
#define BUNDLETRANSFER_H_

namespace dtn
{
	namespace net
	{
		class BundleTransfer {
		public:
			BundleTransfer(const dtn::data::EID &neighbor, const dtn::data::MetaBundle &bundle);
			virtual ~BundleTransfer();

			const dtn::data::EID& getNeighbor() const;
			const dtn::data::MetaBundle& getBundle() const;

			/**
			 * Mark this transmission as aborted
			 */
			void abort(const TransferAbortedEvent::AbortReason reason);

			/**
			 * Mark this transmission as complete
			 */
			void complete();

			/**
			 * Get the number of current transfers
			 */
			static dtn::data::Size count(const dtn::data::EID &neighbor);

		private:
			class Slot {
			public:
				Slot(const dtn::data::EID &neighbor, const dtn::data::MetaBundle &bundle);
				virtual ~Slot();

				const dtn::data::EID neighbor;
				const dtn::data::MetaBundle bundle;

				/**
				 * Mark this transmission as aborted
				 */
				void abort(const TransferAbortedEvent::AbortReason reason);

				/**
				 * Mark this transmission as complete
				 */
				void complete();

				static dtn::data::Size count(const dtn::data::EID &neighbor);

			private:
				static ibrcommon::Mutex _slot_lock;
				typedef std::map<dtn::data::EID, dtn::data::Size> slot_map;
				static slot_map _slot_map;

				bool _completed;
				bool _aborted;
				TransferAbortedEvent::AbortReason _abort_reason;
			};

			refcnt_ptr<Slot> _slot;
		};
	} /* namespace net */
} /* namespace dtn */
#endif /* BUNDLETRANSFER_H_ */
