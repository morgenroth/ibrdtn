/*
 * QRCodeProtocol.h
 *
 * Copyright (C) 2014 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *             Thomas Schrader <schrader.thomas@gmail.com>
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

#ifndef QRCODEPROTOCOL_H_
#define QRCODEPROTOCOL_H_

#include "security/exchange/KeyExchangeProtocol.h"

namespace dtn
{
	namespace security
	{
		class QRCodeProtocol : public dtn::security::KeyExchangeProtocol
		{
		public:
			QRCodeProtocol(KeyExchangeManager &manager);
			virtual ~QRCodeProtocol();

			virtual void begin(KeyExchangeSession &session, KeyExchangeData &data);
			virtual void step(KeyExchangeSession &session, KeyExchangeData &data);
		};
	} /* namespace security */
} /* namespace dtn */

#endif /* QRCODEPROTOCOL_H_ */
