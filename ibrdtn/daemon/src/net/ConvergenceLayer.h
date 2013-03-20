/*
 * ConvergenceLayer.h
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

#ifndef CONVERGENCELAYER_H_
#define CONVERGENCELAYER_H_

#include "ibrdtn/data/BundleID.h"
#include "core/Node.h"

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		class BundleReceiver;

		class NoAddressFoundException : public ibrcommon::Exception
		{
		public:
			NoAddressFoundException(string what = "There is no address available for this peer.") throw() : ibrcommon::Exception(what)
			{
			};
		};

		/**
		 * Ist für die Zustellung von Bundles verantwortlich.
		 */
		class ConvergenceLayer
		{
		public:
			class Job
			{
			public:
				Job();
				Job(const dtn::data::EID &eid, const dtn::data::BundleID &b);
				~Job();

				void clear();

				dtn::data::BundleID _bundle;
				dtn::data::EID _destination;
			};

			ConvergenceLayer();

			/**
			 * destructor
			 */
			virtual ~ConvergenceLayer() = 0;

			virtual dtn::core::Node::Protocol getDiscoveryProtocol() const = 0;

			virtual void queue(const dtn::core::Node &n, const ConvergenceLayer::Job &job) = 0;

			/**
			 * This method opens a connection proactive.
			 * @param n
			 */
			virtual void open(const dtn::core::Node&) {};
		};
	}
}

#endif /*CONVERGENCELAYER_H_*/
