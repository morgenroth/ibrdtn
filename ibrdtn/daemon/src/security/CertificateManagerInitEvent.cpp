/*
 * CertificateManagerInitEvent.cpp
 *
 *  Created on: Apr 3, 2011
 *      Author: roettger
 */

#include "SecurityCertificateManager.h"

namespace dtn {

namespace security {

CertificateManagerInitEvent::CertificateManagerInitEvent(X509 * certificate, EVP_PKEY * privateKey, const ibrcommon::File &trustedCAPath)
	:	certificate(certificate), privateKey(privateKey), trustedCAPath(trustedCAPath)
{
}

CertificateManagerInitEvent::~CertificateManagerInitEvent()
{
}

const std::string
CertificateManagerInitEvent::getName() const
{
	return CertificateManagerInitEvent::className;
}

std::string
CertificateManagerInitEvent::toString() const
{
	return getName();
}

const std::string CertificateManagerInitEvent::className = "CertificateManagerInitEvent";

void
CertificateManagerInitEvent::raise(X509 * certificate, EVP_PKEY * privateKey, const ibrcommon::File &trustedCAPath)
{
	dtn::core::Event::raiseEvent( new CertificateManagerInitEvent(certificate, privateKey, trustedCAPath) );
}

}

}
