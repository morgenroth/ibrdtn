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

			/**
			 * destructor
			 */
			virtual ~ConvergenceLayer() {};

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
