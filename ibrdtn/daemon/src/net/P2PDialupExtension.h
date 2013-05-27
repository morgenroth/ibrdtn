/*
 * P2PDialupExtension.h
 *
 *  Created on: 25.02.2013
 *      Author: morgenro
 */

#include "core/Node.h"
#include <ibrdtn/data/EID.h>
#include <ibrcommon/net/vinterface.h>

#ifndef P2PDIALUPEXTENSION_H_
#define P2PDIALUPEXTENSION_H_

namespace dtn
{
	namespace net
	{
		class P2PDialupExtension {
		public:
			P2PDialupExtension();
			virtual ~P2PDialupExtension() = 0;

			/**
			 * Provides the extension tag used in the node URIs.
			 * E.g. p2p:wifi or p2p:bt
			 */
			virtual dtn::core::Node::Protocol getProtocol() const = 0;

			/**
			 * Try to initiate a connection to a remote peer
			 */
			virtual void connect(const dtn::core::Node::URI &uri) = 0;

			/**
			 * Close an open connection to a peer
			 */
			virtual void disconnect(const dtn::core::Node::URI &uri) = 0;

		protected:
			/**
			 *
			 */
			void fireDiscovered(const dtn::data::EID &eid, const dtn::core::Node::URI &uri) const;

			/**
			 *
			 */
			void fireDisconnected(const dtn::data::EID &eid, const dtn::core::Node::URI &uri) const;

			/**
			 *
			 */
			void fireConnected(const dtn::data::EID &eid, const dtn::core::Node::URI &uri) const;

			/**
			 *
			 */
			void fireInterfaceUp(const ibrcommon::vinterface &iface) const;

			/**
			 *
			 */
			void fireInterfaceDown(const ibrcommon::vinterface &iface) const;
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* P2PDIALUPEXTENSION_H_ */
