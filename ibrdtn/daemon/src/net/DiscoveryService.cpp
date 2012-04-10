/*
 * DiscoveryService.cpp
 *
 *  Created on: 11.09.2009
 *      Author: morgenro
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
		 : _provider(NULL)
		{
		}

		DiscoveryService::DiscoveryService(DiscoveryServiceProvider *provider)
		 : _provider(provider)
		{
		}

		DiscoveryService::DiscoveryService(std::string name, std::string parameters)
		 : _service_name(name), _service_parameters(parameters), _provider(NULL)
		{
		}

		DiscoveryService::~DiscoveryService()
		{
		}

		size_t DiscoveryService::getLength() const
		{
			BundleString name(_service_name);
			BundleString parameters(_service_parameters);

			return name.getLength() + parameters.getLength();
		}

		std::string DiscoveryService::getName() const
		{
			return _service_name;
		}

		std::string DiscoveryService::getParameters() const
		{
			return _service_parameters;
		}

		void DiscoveryService::update(const ibrcommon::vinterface &net)
		{
			if (_provider != NULL) _provider->update(net, _service_name, _service_parameters);
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
