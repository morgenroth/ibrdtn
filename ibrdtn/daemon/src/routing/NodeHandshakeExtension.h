/*
 * NodeHandshakeExtension.h
 *
 *  Created on: 08.12.2011
 *      Author: morgenro
 */

#ifndef NODEHANDSHAKEEXTENSION_H_
#define NODEHANDSHAKEEXTENSION_H_

#include "routing/BaseRouter.h"
#include "core/AbstractWorker.h"

#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/Queue.h>
#include <map>

namespace dtn
{
	namespace routing
	{
		class NodeHandshakeExtension : public BaseRouter::Extension
		{
		public:
			NodeHandshakeExtension();
			virtual ~NodeHandshakeExtension();

			void notify(const dtn::core::Event *evt);

			void doHandshake(const dtn::data::EID &eid);

			/**
			 * @see BaseRouter::requestHandshake()
			 */
			void requestHandshake(const dtn::data::EID &destination, NodeHandshake &request) const;

			/**
			 * @see BaseRouter::responseHandshake()
			 */
			void responseHandshake(const dtn::data::EID &source, const NodeHandshake &request, NodeHandshake &answer);

			/**
			 * @see BaseRouter::processHandshake()
			 */
			void processHandshake(const dtn::data::EID &source, NodeHandshake &answer);

		protected:
			void processHandshake(const dtn::data::Bundle &bundle);
			const std::list<BaseRouter::Extension*>& getExtensions();

		private:
			class HandshakeEndpoint : public dtn::core::AbstractWorker
			{
			public:
				HandshakeEndpoint(NodeHandshakeExtension &callback);
				virtual ~HandshakeEndpoint();

				void callbackBundleReceived(const Bundle &b);
				void query(const dtn::data::EID &eid);

				void send(const dtn::data::Bundle &b);

				void removeFromBlacklist(const dtn::data::EID &eid);

			private:
				NodeHandshakeExtension &_callback;
				std::map<dtn::data::EID, size_t> _blacklist;
				ibrcommon::Mutex _blacklist_lock;
			};

			/**
			 * process the incoming bundles and send messages to other routers
			 */
			HandshakeEndpoint _endpoint;
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* NODEHANDSHAKEEXTENSION_H_ */
