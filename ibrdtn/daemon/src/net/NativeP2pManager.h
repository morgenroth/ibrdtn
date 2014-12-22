/*
 * NativeP2pManager.h
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

#ifndef NATIVEP2PMANAGER_H_
#define NATIVEP2PMANAGER_H_

#include "net/P2PDialupExtension.h"
#include "core/Node.h"
#include <ibrdtn/data/EID.h>
#include <string>

namespace dtn
{
	namespace net
	{
		class NativeP2pManager : protected dtn::net::P2PDialupExtension
		{
			// the protocol returned by getProtocol()
			const dtn::core::Node::Protocol _protocol;

		public:
			NativeP2pManager(const std::string &protocol);
			virtual ~NativeP2pManager() = 0;

			/**
			 * @see P2PDialupExtension::getProtocol()
			 */
			virtual dtn::core::Node::Protocol getProtocol() const;

			/**
			 * @see P2PDialupExtension::connect()
			 */
			virtual void connect(const dtn::core::Node::URI &uri);

			/**
			 * @see P2PDialupExtension::disconnect()
			 */
			virtual void disconnect(const dtn::core::Node::URI &uri);

			// method for JNI
			virtual void connect(const std::string &identifier) = 0;

			// method for JNI
			virtual void disconnect(const std::string &identifier) = 0;

		protected:
			// method for JNI
			virtual void fireDiscovered(const dtn::data::EID &eid, const std::string &identifier, size_t timeout);

			// method for JNI
			virtual void fireDisconnected(const dtn::data::EID &eid, const std::string &identifier);

			// method for JNI
			virtual void fireConnected(const dtn::data::EID &eid, const std::string &identifier, size_t timeout);

			// method for JNI
			virtual void fireInterfaceUp(const std::string &iface);

			// method for JNI
			virtual void fireInterfaceDown(const std::string &iface);
		};
	} /* namespace net */
} /* namespace dtn */
#endif /* NATIVEP2PMANAGER_H_ */
