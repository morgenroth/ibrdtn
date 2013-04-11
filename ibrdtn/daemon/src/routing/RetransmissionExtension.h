/*
 * RetransmissionExtension.h
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

#ifndef RETRANSMISSIONEXTENSION_H_
#define RETRANSMISSIONEXTENSION_H_

#include "routing/RoutingExtension.h"
#include <ibrdtn/data/BundleID.h>
#include <ibrdtn/data/EID.h>
#include <ibrcommon/thread/Mutex.h>
#include <queue>
#include <set>

namespace dtn
{
	namespace routing
	{
		class RetransmissionExtension : public RoutingExtension
		{
		public:
			RetransmissionExtension();
			virtual ~RetransmissionExtension();

			void notify(const dtn::core::Event *evt) throw ();
			void componentUp() throw () {};
			void componentDown() throw () {};

		private:
			class RetransmissionData : public dtn::data::BundleID
			{
			public:
				RetransmissionData(const dtn::data::BundleID &id, dtn::data::EID destination, size_t retry = 2);
				virtual ~RetransmissionData();


				const dtn::data::EID destination;

				size_t getTimestamp() const;
				size_t getCount() const;

				RetransmissionData& operator++();
				RetransmissionData& operator++(int);

				bool operator!=(const RetransmissionData &obj);
				bool operator==(const RetransmissionData &obj);

			private:
				size_t _timestamp;
				size_t _count;
				const size_t retry;
			};

			ibrcommon::Mutex _mutex;
			std::queue<RetransmissionData> _queue;
			std::set<RetransmissionData> _set;
		};
	}
}

#endif /* RETRANSMISSIONEXTENSION_H_ */
