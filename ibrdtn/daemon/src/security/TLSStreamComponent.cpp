/*
 * TLSStreamComponent.cpp
 *
 *  Created on: Apr 2, 2011
 *      Author: roettger
 */

#include "security/TLSStreamComponent.h"

#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>
#include <ibrcommon/net/TLSStream.h>
#include "SecurityCertificateManager.h"
#include "Configuration.h"

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
		TLSStreamComponent::raiseEvent(const dtn::core::Event *evt)
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
			bindEvent(CertificateManagerInitEvent::className);
		}

		void
		TLSStreamComponent::startup()
		{
			/* nothing to do */
		}

		void
		TLSStreamComponent::terminate()
		{
			unbindEvent(CertificateManagerInitEvent::className);
		}

		const std::string
		TLSStreamComponent::getName() const
		{
			return "TLSStreamComponent";
		}
	}
}
