/*
 * KeyExchangeEvent.h
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

#ifndef KEYEXCHANGEEVENT_H_
#define KEYEXCHANGEEVENT_H_

#include "core/Event.h"
#include "ibrdtn/data/EID.h"

#include "security/exchange/KeyExchangeData.h"

#include <string>

namespace dtn
{
	namespace security
	{
		class KeyExchangeEvent : public dtn::core::Event
		{
			public:
				virtual ~KeyExchangeEvent();

				const dtn::data::EID& getEID() const;
				const dtn::security::KeyExchangeData& getData() const;

				const std::string getName() const;
				std::string getMessage() const;

				static void raise(const dtn::data::EID &eid, const dtn::security::KeyExchangeData &data);

				static const std::string className;

			private:

				KeyExchangeEvent(const dtn::data::EID &eid, const dtn::security::KeyExchangeData &data);

				const dtn::data::EID _eid;
				const dtn::security::KeyExchangeData _data;
		};

	} /* namespace security */
} /* namespace dtn */

#endif /* KEYEXCHANGEEVENT_H_ */
