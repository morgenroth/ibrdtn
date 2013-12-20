/*
 * DiscoveryService.cpp
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

#include "net/DiscoveryService.h"
#include "ibrdtn/data/SDNV.h"
#include "ibrdtn/data/BundleString.h"
#include <string>

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		DiscoveryService::DiscoveryService()
		 : _service_protocol(dtn::core::Node::CONN_UNDEFINED)
		{
		}

		DiscoveryService::DiscoveryService(const dtn::core::Node::Protocol p, const std::string &parameters)
		 : _service_protocol(p), _service_name(asTag(p)), _service_parameters(parameters)
		{
		}

		DiscoveryService::DiscoveryService(const std::string &name, const std::string &parameters)
		 : _service_protocol(asProtocol(name)), _service_name(name), _service_parameters(parameters)
		{
		}

		DiscoveryService::~DiscoveryService()
		{
		}

		dtn::data::Length DiscoveryService::getLength() const
		{
			BundleString name(_service_name);
			BundleString parameters(_service_parameters);

			return name.getLength() + parameters.getLength();
		}

		dtn::core::Node::Protocol DiscoveryService::getProtocol() const
		{
			return _service_protocol;
		}

		const std::string& DiscoveryService::getName() const
		{
			return _service_name;
		}

		const std::string& DiscoveryService::getParameters() const
		{
			return _service_parameters;
		}

		void DiscoveryService::update(const std::string &parameters)
		{
			_service_parameters = parameters;
		}

		dtn::core::Node::Protocol DiscoveryService::asProtocol(const std::string &tag)
		{
			if (tag == "undefined") {
				return dtn::core::Node::CONN_UNDEFINED;
			}
			else if (tag == "udpcl") {
				return dtn::core::Node::CONN_UDPIP;
			}
			else if (tag == "tcpcl") {
				return dtn::core::Node::CONN_TCPIP;
			}
			else if (tag == "lowpancl") {
				return dtn::core::Node::CONN_LOWPAN;
			}
			else if (tag == "bt") {
				return dtn::core::Node::CONN_BLUETOOTH;
			}
			else if (tag == "http") {
				return dtn::core::Node::CONN_HTTP;
			}
			else if (tag == "file") {
				return dtn::core::Node::CONN_FILE;
			}
			else if (tag == "dgram:udp") {
				return dtn::core::Node::CONN_DGRAM_UDP;
			}
			else if (tag == "dgram:eth") {
				return dtn::core::Node::CONN_DGRAM_ETHERNET;
			}
			else if (tag == "dgram:lowpan") {
				return dtn::core::Node::CONN_DGRAM_LOWPAN;
			}
			else if (tag == "p2p:wifi") {
				return dtn::core::Node::CONN_P2P_WIFI;
			}
			else if (tag == "p2p:bt") {
				return dtn::core::Node::CONN_P2P_BT;
			}
			else if (tag == "email") {
				return dtn::core::Node::CONN_EMAIL;
			}

			return dtn::core::Node::CONN_UNSUPPORTED;
		}

		std::string DiscoveryService::asTag(const dtn::core::Node::Protocol proto)
		{
			switch (proto)
			{
			case dtn::core::Node::CONN_UNSUPPORTED:
				return "unsupported";

			case dtn::core::Node::CONN_UNDEFINED:
				return "undefined";

			case dtn::core::Node::CONN_UDPIP:
				return "udpcl";

			case dtn::core::Node::CONN_TCPIP:
				return "tcpcl";

			case dtn::core::Node::CONN_LOWPAN:
				return "lowpancl";

			case dtn::core::Node::CONN_BLUETOOTH:
				return "bt";

			case dtn::core::Node::CONN_HTTP:
				return "http";

			case dtn::core::Node::CONN_FILE:
				return "file";

			case dtn::core::Node::CONN_DGRAM_UDP:
				return "dgram:udp";

			case dtn::core::Node::CONN_DGRAM_ETHERNET:
				return "dgram:eth";

			case dtn::core::Node::CONN_DGRAM_LOWPAN:
				return "dgram:lowpan";

			case dtn::core::Node::CONN_P2P_WIFI:
				return "p2p:wifi";

			case dtn::core::Node::CONN_P2P_BT:
				return "p2p:bt";

			case dtn::core::Node::CONN_EMAIL:
				return "email";
			}

			return "unknown";
		}

		std::ostream &operator<<(std::ostream &stream, const DiscoveryService &service)
		{
			BundleString name(service._service_name);
			BundleString parameters(service._service_parameters);

			stream << name << parameters;

			return stream;
		}

		std::istream &operator>>(std::istream &stream, DiscoveryService &service)
		{
			BundleString name;
			BundleString parameters;

			stream >> name >> parameters;

			service._service_protocol = DiscoveryService::asProtocol((const std::string&)name);
			service._service_name = (const std::string&)name;
			service._service_parameters = (const std::string&)parameters;

			return stream;
		}
	}
}
