#ifndef BUNDLECORE_H_
#define BUNDLECORE_H_

#include "Component.h"

#include "core/EventReceiver.h"
#include "core/StatusReportGenerator.h"
#include "storage/BundleStorage.h"
#include "core/WallClock.h"
#include "routing/BaseRouter.h"

#include "net/ConnectionManager.h"
#include "net/ConvergenceLayer.h"

#include <ibrdtn/data/Serializer.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/CustodySignalBlock.h>

#include <vector>
#include <set>
#include <map>

using namespace dtn::data;

namespace dtn
{
	namespace core
	{
		/**
		 * The BundleCore manage the Bundle Protocol basics
		 */
		class BundleCore : public dtn::daemon::IntegratedComponent, public dtn::core::EventReceiver, public dtn::data::Validator
		{
		public:
			static dtn::data::EID local;

			static BundleCore& getInstance();

			WallClock& getClock();

			void setStorage(dtn::storage::BundleStorage *storage);
			dtn::storage::BundleStorage& getStorage();

			void setRouter(dtn::routing::BaseRouter *router);
			dtn::routing::BaseRouter& getRouter() const;

			void transferTo(const dtn::data::EID &destination, const dtn::data::BundleID &bundle);

			void addConvergenceLayer(dtn::net::ConvergenceLayer *cl);

			void addConnection(const dtn::core::Node &n);
			void removeConnection(const dtn::core::Node &n);

			/**
			 * Add a static route to the static routing module.
			 * @param destination
			 * @param nexthop
			 * @param timeout
			 */
			void addRoute(const dtn::data::EID &destination, const dtn::data::EID &nexthop, size_t timeout = 0);

			/**
			 * Remove a static route from the static routing module.
			 * @param destination
			 * @param nexthop
			 */
			void removeRoute(const dtn::data::EID &destination, const dtn::data::EID &nexthop);

			/**
			 * get a set with all neighbors
			 * @return
			 */
			const std::set<dtn::core::Node> getNeighbors();

			/**
			 * Checks if a node is already known as neighbor.
			 * @param
			 * @return
			 */
			bool isNeighbor(const dtn::core::Node &node);

			/**
			 * Get the neighbor with the given EID.
			 * @throw dtn::net::NeighborNotAvailableException if the neighbor is not available.
			 * @param eid The EID of the neighbor.
			 * @return A node object with all neighbor data.
			 */
			const dtn::core::Node getNeighbor(const dtn::data::EID &eid);

			void raiseEvent(const dtn::core::Event *evt);

			virtual void validate(const dtn::data::PrimaryBlock &obj) const throw (RejectedException);
			virtual void validate(const dtn::data::Block &obj, const size_t length) const throw (RejectedException);
			virtual void validate(const dtn::data::Bundle &obj) const throw (RejectedException);

			/**
			 * Define a global block size limit. This is used in the validator to reject bundles while receiving.
			 */
			static size_t blocksizelimit;

			/**
			 * Define the maximum lifetime for accepted bundles
			 */
			static size_t max_lifetime;

			/**
			 * Define the maximum offset for the timestamp of pre-dated bundles
			 */
			static size_t max_timestamp_future;

			/**
			 * Define if forwarding is allowed. If set to false, this daemon only accepts bundles for local applications.
			 */
			static bool forwarding;

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

			static void processBlocks(dtn::data::Bundle &b);

		protected:
			virtual void componentUp();
			virtual void componentDown();

		private:
			/**
			 * default constructor
			 */
			BundleCore();

			/**
			 * destructor
			 */
			virtual ~BundleCore();

			/**
			 * Forbidden copy constructor
			 */
			BundleCore operator=(const BundleCore &k) { return k; };

			/**
			 * This is a clock object. It can be used to synchronize methods to the local clock.
			 */
			WallClock _clock;

			dtn::storage::BundleStorage *_storage;
			dtn::routing::BaseRouter *_router;

			// generator for statusreports
			StatusReportGenerator _statusreportgen;

			// manager class for connections
			dtn::net::ConnectionManager _connectionmanager;
		};
	}
}

#endif /*BUNDLECORE_H_*/

