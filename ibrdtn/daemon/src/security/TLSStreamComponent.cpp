/*
 * TLSStreamComponent.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Stephen Roettger <roettger@ibr.cs.tu-bs.de>
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

#include "Configuration.h"
#include "core/EventDispatcher.h"
#include "security/SecurityCertificateManager.h"
#include "security/TLSStreamComponent.h"

#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>
#include <ibrcommon/ssl/TLSStream.h>


namespace dtn
{
	namespace security
	{
		TLSStreamComponent::TLSStreamComponent()
		{
		}

		TLSStreamComponent::~TLSStreamComponent()
		{
		}

		void
		TLSStreamComponent::raiseEvent(const dtn::core::Event *evt) throw ()
		{
			if(evt->getName() == CertificateManagerInitEvent::className)
			{
				try {
					const CertificateManagerInitEvent *event = dynamic_cast<const CertificateManagerInitEvent*>(evt);
					if (event)
					{
						ibrcommon::TLSStream::init(event->certificate, event->privateKey, event->trustedCAPath, !dtn::daemon::Configuration::getInstance().getSecurity().TLSEncryptionDisabled());
					}
				} catch(const ibrcommon::Exception&) { }
			}
		}

		/* functions from Component */
		void
		TLSStreamComponent::initialize()
		{
			dtn::core::EventDispatcher<CertificateManagerInitEvent>::add(this);
		}

		void
		TLSStreamComponent::startup()
		{
			/* nothing to do */
		}

		void
		TLSStreamComponent::terminate()
		{
			dtn::core::EventDispatcher<CertificateManagerInitEvent>::remove(this);
		}

		const std::string
		TLSStreamComponent::getName() const
		{
			return "TLSStreamComponent";
		}
	}
}
