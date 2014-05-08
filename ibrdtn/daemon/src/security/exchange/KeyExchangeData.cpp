/*
 * KeyExchangeData.cpp
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

#include "security/exchange/KeyExchangeData.h"
#include <ibrdtn/data/BundleString.h>

namespace dtn
{
	namespace security
	{
		KeyExchangeData::KeyExchangeData()
		 : _session_id(-1), _action(ERROR), _protocol(-1), _step(-1)
		{}

		KeyExchangeData::KeyExchangeData(const Action action, const KeyExchangeSession &session)
		 : _session_id(session.getUniqueId()), _action(action), _protocol(session.getProtocol()), _step(0)
		{}

		KeyExchangeData::KeyExchangeData(const Action action, const int protocol)
		 : _session_id(-1), _action(action), _protocol(protocol), _step(0)
		{
		}

		KeyExchangeData::~KeyExchangeData()
		{}

		KeyExchangeData::KeyExchangeData(const KeyExchangeData &obj)
		 : _session_id(obj._session_id), _action(obj._action), _protocol(obj._protocol), _step(obj._step)
		{
			str(obj.str());
		}

		std::string KeyExchangeData::toString() const
		{
			std::stringstream sstm;

			switch (_protocol)
			{
			case 0:
				sstm << "none";
				break;
			case 1:
				sstm << "dh";
				break;
			case 2:
				sstm << "jpake";
				break;
			case 3:
				sstm << "hash";
				break;
			case 4:
				sstm << "password";
				break;
			case 5:
				sstm << "qr";
				break;
			case 6:
				sstm << "nfc";
				break;
			}

			sstm << "::" << _session_id;
			return sstm.str();
		}

		KeyExchangeData::Action KeyExchangeData::getAction() const
		{
			return _action;
		}

		void KeyExchangeData::setAction(Action action)
		{
			_action = action;
		}

		int KeyExchangeData::getProtocol() const
		{
			return _protocol;
		}

		void KeyExchangeData::setProtocol(int protocol)
		{
			_protocol = protocol;
		}

		void KeyExchangeData::setSession(const KeyExchangeSession &session)
		{
			_session_id = session.getUniqueId();
		}

		void KeyExchangeData::setSessionId(unsigned int sessionId)
		{
			_session_id = sessionId;
		}

		unsigned int KeyExchangeData::getSessionId() const
		{
			return _session_id;
		}

		int KeyExchangeData::getStep() const
		{
			return _step;
		}

		void KeyExchangeData::setStep(int step)
		{
			_step = step;
		}

		KeyExchangeData& KeyExchangeData::operator=(const KeyExchangeData &obj)
		{
			_session_id = obj._session_id;
			_action = obj._action;
			_protocol = obj._protocol;
			_step = obj._step;
			str(obj.str());
			return *this;
		}

		std::ostream &operator<<(std::ostream &stream, const KeyExchangeData &obj)
		{
			stream << dtn::data::Number(obj._session_id);

			stream << (char)obj._action;

			stream << dtn::data::Number(obj._protocol);
			stream << dtn::data::Number(obj._step);

			stream << dtn::data::BundleString(obj.str());

			return stream;
		}

		std::istream &operator>>(std::istream &stream, KeyExchangeData &obj)
		{
			char action = 0;
			dtn::data::BundleString bs;
			dtn::data::Number sdnv;

			stream >> sdnv;
			obj._session_id = sdnv.get<unsigned int>();

			stream >> action;
			obj._action = KeyExchangeData::Action(action);

			stream >> sdnv;
			obj._protocol = sdnv.get<int>();

			stream >> sdnv;
			obj._step = sdnv.get<int>();

			stream >> bs;
			obj.str(bs);

			return stream;
		}
	} /* namespace security */
} /* namespace dtn */
