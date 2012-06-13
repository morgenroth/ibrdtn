/*
 * EventConnection.cpp
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

#include "EventConnection.h"

#include "core/NodeEvent.h"
#include "core/GlobalEvent.h"
#include "core/CustodyEvent.h"
#include "routing/QueueBundleEvent.h"
#include "net/TransferAbortedEvent.h"
#include "net/TransferCompletedEvent.h"
#include "net/ConnectionEvent.h"

#include <ibrdtn/utils/Utils.h>

namespace dtn
{
	namespace api
	{
		EventConnection::EventConnection(ClientHandler &client, ibrcommon::tcpstream &stream)
		 : ProtocolHandler(client, stream), _running(true)
		{
		}

		EventConnection::~EventConnection()
		{
		}

		void EventConnection::raiseEvent(const dtn::core::Event *evt)
		{
			ibrcommon::MutexLock l(_mutex);
			if (!_running) return;

			try {
				const dtn::core::NodeEvent &node = dynamic_cast<const dtn::core::NodeEvent&>(*evt);

				// ignore NODE_INFO_UPDATED
				if (node.getAction() == dtn::core::NODE_INFO_UPDATED) return;

				// start with the event tag
				_stream << "Event: " << node.getName() << std::endl;
				_stream << "Action: ";

				switch (node.getAction())
				{
				case dtn::core::NODE_AVAILABLE:
					_stream << "available";
					break;
				case dtn::core::NODE_UNAVAILABLE:
					_stream << "unavailable";
					break;
				default:
					break;
				}

				_stream << std::endl;

				// write the node eid
				_stream << "EID: " << node.getNode().getEID().getString() << std::endl;

				// close the event
				_stream << std::endl;
			} catch (const std::bad_cast&) { };

			try {
				const dtn::core::GlobalEvent &global = dynamic_cast<const dtn::core::GlobalEvent&>(*evt);

				// start with the event tag
				_stream << "Event: " << global.getName() << std::endl;
				_stream << "Action: ";

				switch (global.getAction())
				{
				case dtn::core::GlobalEvent::GLOBAL_BUSY:
					_stream << "busy";
					break;
				case dtn::core::GlobalEvent::GLOBAL_IDLE:
					_stream << "idle";
					break;
				case dtn::core::GlobalEvent::GLOBAL_POWERSAVE:
					_stream << "powersave";
					break;
				case dtn::core::GlobalEvent::GLOBAL_RELOAD:
					_stream << "reload";
					break;
				case dtn::core::GlobalEvent::GLOBAL_SHUTDOWN:
					_stream << "shutdown";
					break;
				case dtn::core::GlobalEvent::GLOBAL_SUSPEND:
					_stream << "suspend";
					break;
				default:
					break;
				}
				_stream << std::endl;

				// close the event
				_stream << std::endl;
			} catch (const std::bad_cast&) { };

			try {
				const dtn::core::CustodyEvent &custody = dynamic_cast<const dtn::core::CustodyEvent&>(*evt);

			} catch (const std::bad_cast&) { };

			try {
				const dtn::net::TransferAbortedEvent &aborted = dynamic_cast<const dtn::net::TransferAbortedEvent&>(*evt);

			} catch (const std::bad_cast&) { };

			try {
				const dtn::net::TransferCompletedEvent &completed = dynamic_cast<const dtn::net::TransferCompletedEvent&>(*evt);

			} catch (const std::bad_cast&) { };

			try {
				const dtn::net::ConnectionEvent &connection = dynamic_cast<const dtn::net::ConnectionEvent&>(*evt);

				// start with the event tag
				_stream << "Event: " << connection.getName() << std::endl;
				_stream << "Action: ";

				switch (connection.state)
				{
				case dtn::net::ConnectionEvent::CONNECTION_UP:
					_stream << "up";
					break;
				case dtn::net::ConnectionEvent::CONNECTION_DOWN:
					_stream << "down";
					break;
				case dtn::net::ConnectionEvent::CONNECTION_SETUP:
					_stream << "setup";
					break;
				case dtn::net::ConnectionEvent::CONNECTION_TIMEOUT:
					_stream << "timeout";
					break;
				default:
					break;
				}
				_stream << std::endl;

				// write the peer eid
				_stream << "Peer: " << connection.peer.getString() << std::endl;

				// close the event
				_stream << std::endl;
			} catch (const std::bad_cast&) { };

			try {
				const dtn::routing::QueueBundleEvent &queued = dynamic_cast<const dtn::routing::QueueBundleEvent&>(*evt);

				// start with the event tag
				_stream << "Event: " << queued.getName() << std::endl;

				// write the bundle data
				_stream << "Source: " << queued.bundle.source.getString() << std::endl;
				_stream << "Timestamp: " << queued.bundle.timestamp << std::endl;
				_stream << "Sequencenumber: " << queued.bundle.sequencenumber << std::endl;
				_stream << "Lifetime: " << queued.bundle.lifetime << std::endl;
				_stream << "Procflags: " << queued.bundle.procflags << std::endl;

				// write the destination eid
				_stream << "Destination: " << queued.bundle.destination.getString() << std::endl;

				if (queued.bundle.fragment)
				{
					// write fragmentation values
					_stream << "Appdatalength: " << queued.bundle.appdatalength << std::endl;
					_stream << "Fragmentoffset: " << queued.bundle.offset << std::endl;
				}

				// close the event
				_stream << std::endl;
			} catch (const std::bad_cast&) { };
		}

		void EventConnection::run()
		{
			std::string buffer;

			// announce protocol change
			_stream << ClientHandler::API_STATUS_OK << " SWITCHED TO EVENT" << std::endl;

			// run as long the stream is ok
			while (_stream.good())
			{
				getline(_stream, buffer);

				// search for '\r\n' and remove the '\r'
				std::string::reverse_iterator iter = buffer.rbegin();
				if ( (*iter) == '\r' ) buffer = buffer.substr(0, buffer.length() - 1);

				std::vector<std::string> cmd = dtn::utils::Utils::tokenize(" ", buffer);
				if (cmd.size() == 0) continue;

				// return to previous level
				if (cmd[0] == "exit") break;
			}

			ibrcommon::MutexLock l(_mutex);
			_running = false;
		}

		void EventConnection::setup()
		{
			// bind to several events
			bindEvent(dtn::core::NodeEvent::className);
			bindEvent(dtn::core::GlobalEvent::className);
			bindEvent(dtn::core::CustodyEvent::className);
			bindEvent(dtn::net::TransferAbortedEvent::className);
			bindEvent(dtn::net::TransferCompletedEvent::className);
			bindEvent(dtn::net::ConnectionEvent::className);
			bindEvent(dtn::routing::QueueBundleEvent::className);
		}

		void EventConnection::finally()
		{
			// unbind to events
			unbindEvent(dtn::core::NodeEvent::className);
			unbindEvent(dtn::core::GlobalEvent::className);
			unbindEvent(dtn::core::CustodyEvent::className);
			unbindEvent(dtn::net::TransferAbortedEvent::className);
			unbindEvent(dtn::net::TransferCompletedEvent::className);
			unbindEvent(dtn::net::ConnectionEvent::className);
			unbindEvent(dtn::routing::QueueBundleEvent::className);
		}

		void EventConnection::__cancellation()
		{
		}
	} /* namespace api */
} /* namespace dtn */
