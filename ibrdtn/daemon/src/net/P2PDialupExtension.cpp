/*
 * P2PDialupExtension.cpp
 *
 *  Created on: 25.02.2013
 *      Author: morgenro
 */

#include "net/P2PDialupExtension.h"
#include "net/P2PDialupEvent.h"
#include "core/BundleCore.h"
#include "core/NodeEvent.h"

namespace dtn
{
	namespace net
	{

		P2PDialupExtension::P2PDialupExtension() {
			// register at the bundle core
			dtn::core::BundleCore::getInstance().getConnectionManager().add(this);
		}

		P2PDialupExtension::~P2PDialupExtension() {
			// unregister at the bundle core
			dtn::core::BundleCore::getInstance().getConnectionManager().remove(this);
		}

		void P2PDialupExtension::fireDiscovered(const dtn::data::EID &eid, const dtn::core::Node::URI &uri) const
		{
			dtn::net::ConnectionManager &cm = dtn::core::BundleCore::getInstance().getConnectionManager();
			dtn::core::Node n(eid);
			n.add(uri);
			cm.add(n);
		}

		void P2PDialupExtension::fireDisconnected(const dtn::data::EID &eid, const dtn::core::Node::URI &uri) const
		{
			dtn::net::ConnectionManager &cm = dtn::core::BundleCore::getInstance().getConnectionManager();
			dtn::core::Node n(eid);
			n.add(uri);
			cm.remove(n);
		}

		void P2PDialupExtension::fireConnected(const dtn::data::EID &eid, const dtn::core::Node::URI &uri) const
		{
			dtn::net::ConnectionManager &cm = dtn::core::BundleCore::getInstance().getConnectionManager();
			dtn::core::Node n(eid);
			n.add(uri);
			cm.add(n);
		}

		void P2PDialupExtension::fireInterfaceUp(const ibrcommon::vinterface &iface) const
		{
			P2PDialupEvent::raise(P2PDialupEvent::INTERFACE_UP, iface);
		}

		void P2PDialupExtension::fireInterfaceDown(const ibrcommon::vinterface &iface) const
		{
			P2PDialupEvent::raise(P2PDialupEvent::INTERFACE_DOWN, iface);
		}
	} /* namespace routing */
} /* namespace dtn */
