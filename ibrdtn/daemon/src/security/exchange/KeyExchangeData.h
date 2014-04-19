/*
 * KeyExchangeData.h
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

#ifndef KEYEXCHANGEDATA_H_
#define KEYEXCHANGEDATA_H_

#include "security/exchange/KeyExchangeSession.h"

#include <string>
#include <sstream>
#include <iostream>

namespace dtn
{
	namespace security
	{

		class KeyExchangeData : public std::stringstream
		{
			public:
				enum Action
				{
					START = 0,
					REQUEST = 1,
					RESPONSE = 2,
					COMPLETE = 3,
					PASSWORD_REQUEST = 4,
					WRONG_PASSWORD = 5,
					HASH_COMMIT = 6,
					NEWKEY_FOUND = 7,
					ERROR = 8
				};

				KeyExchangeData();
				KeyExchangeData(const Action action, const KeyExchangeSession &session);
				KeyExchangeData(const Action action, const int protocol);
				virtual ~KeyExchangeData();

				KeyExchangeData(const KeyExchangeData&);

				KeyExchangeData& operator=(const KeyExchangeData&);

				int getProtocol() const;
				void setProtocol(int protocol);

				Action getAction() const;
				void setAction(Action action);

				void setSession(const KeyExchangeSession &session);
				void setSessionId(unsigned int sessionId);
				unsigned int getSessionId() const;

				int getStep() const;
				void setStep(int step);

				friend std::ostream &operator<<(std::ostream &stream, const KeyExchangeData &obj);
				friend std::istream &operator>>(std::istream &stream, KeyExchangeData &obj);

				std::string toString() const;

			private:
				unsigned int _session_id;
				Action _action;
				int _protocol;
				int _step;
		};

	} /* namespace security */
} /* namespace dtn */
#endif /* KEYEXCHANGEDATA_H_ */
