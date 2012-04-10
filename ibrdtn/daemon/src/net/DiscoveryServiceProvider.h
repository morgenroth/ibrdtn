/* 
 * File:   DiscoveryServiceProvider.h
 * Author: morgenro
 *
 * Created on 17. November 2009, 17:27
 */

#ifndef _DISCOVERYSERVICEPROVIDER_H
#define	_DISCOVERYSERVICEPROVIDER_H

#include <ibrcommon/net/vinterface.h>
#include <string>

namespace dtn
{
	namespace net
	{
		class DiscoveryServiceProvider
		{
		public:
			class NoServiceHereException : public ibrcommon::Exception
			{
			public:
				NoServiceHereException(string what = "No service available.") throw() : ibrcommon::Exception(what)
				{
				};

				virtual ~NoServiceHereException() throw() {};
			};

			/**
			 * Updates an discovery service block with current values
			 * @param name
			 * @param data
			 */
			virtual void update(const ibrcommon::vinterface &iface, std::string &name, std::string &data)
				throw(NoServiceHereException) = 0;
		};
	}
}

#endif	/* _DISCOVERYSERVICEPROVIDER_H */

