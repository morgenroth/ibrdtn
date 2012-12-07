/*
 * CertificateManagerInitEvent.cpp
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

#include "SecurityCertificateManager.h"
#include "core/EventDispatcher.h"

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
	dtn::core::EventDispatcher<CertificateManagerInitEvent>::raise( new CertificateManagerInitEvent(certificate, privateKey, trustedCAPath) );
}

}

}
