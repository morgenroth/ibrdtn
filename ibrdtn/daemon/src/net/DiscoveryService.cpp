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
		{
		}

		DiscoveryService::DiscoveryService(std::string name, std::string parameters)
		 : _service_name(name), _service_parameters(parameters)
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

		const std::string& DiscoveryService::getName() const
		{
			return _service_name;
		}

		const std::string& DiscoveryService::getParameters() const
		{
			return _service_parameters;
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

			service._service_name = (std::string)name;
			service._service_parameters = (std::string)parameters;

			return stream;
		}
	}
}
