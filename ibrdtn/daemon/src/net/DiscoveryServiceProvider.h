/* 
 * DiscoveryServiceProvider.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
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

