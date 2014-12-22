/*
 * NativeP2pManager.cpp
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

#include "net/NativeP2pManager.h"

namespace dtn
{
	namespace net
	{
		NativeP2pManager::NativeP2pManager(const std::string &protocol)
		: _protocol(dtn::core::Node::fromProtocolString(protocol))
		{
		}

		NativeP2pManager::~NativeP2pManager()
		{
		}

		void NativeP2pManager::fireDiscovered(const dtn::data::EID &eid, const std::string &identifier, size_t timeout)
		{
			// create a p2p uri for the discovered node
			const dtn::core::Node::URI uri(dtn::core::Node::NODE_P2P_DIALUP, getProtocol(), identifier, timeout, -50);

			// announce the discovered peer
			P2PDialupExtension::fireDiscovered(eid, uri);
		}

		void NativeP2pManager::fireDisconnected(const dtn::data::EID &eid, const std::string &identifier)
		{
			// create a p2p uri for the discovered node
			const dtn::core::Node::URI uri(dtn::core::Node::NODE_CONNECTED, getProtocol(), identifier, 0);

			// announce the disconnected peer
			P2PDialupExtension::fireDisconnected(eid, uri);
		}

		void NativeP2pManager::fireConnected(const dtn::data::EID &eid, const std::string &identifier, size_t timeout)
		{
			// create a p2p uri for the discovered node
			const dtn::core::Node::URI uri(dtn::core::Node::NODE_CONNECTED, getProtocol(), identifier, timeout, -40);

			// announce the connected peer
			P2PDialupExtension::fireConnected(eid, uri);
		}

		void NativeP2pManager::fireInterfaceUp(const std::string &i)
		{
			// create a new interface object
			const ibrcommon::vinterface iface(i);

			// fire the interface event
			P2PDialupExtension::fireInterfaceUp(iface);
		}

		void NativeP2pManager::fireInterfaceDown(const std::string &i)
		{
			// create a new interface object
			const ibrcommon::vinterface iface(i);

			// fire the interface event
			P2PDialupExtension::fireInterfaceDown(iface);
		}

		dtn::core::Node::Protocol NativeP2pManager::getProtocol() const
		{
			return _protocol;
		}

		void NativeP2pManager::connect(const dtn::core::Node::URI &uri)
		{
			connect(uri.value);
		}

		void NativeP2pManager::disconnect(const dtn::core::Node::URI &uri)
		{
			disconnect(uri.value);
		}
	} /* namespace net */
} /* namespace dtn */
