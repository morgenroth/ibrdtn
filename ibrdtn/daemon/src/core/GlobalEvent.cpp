/*
 * GlobalEvent.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
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

#include "core/GlobalEvent.h"
#include "core/EventDispatcher.h"
using namespace std;

namespace dtn
{
	namespace core
	{
		GlobalEvent::GlobalEvent(const Action a)
		 : _action(a)
		{
		}

		GlobalEvent::~GlobalEvent()
		{
		}

		GlobalEvent::Action GlobalEvent::getAction() const
		{
			return _action;
		}

		const std::string GlobalEvent::getName() const
		{
			return GlobalEvent::className;
		}

		void GlobalEvent::raise(const Action a)
		{
			// raise the new event
			dtn::core::EventDispatcher<GlobalEvent>::raise( new GlobalEvent(a) );
		}

		const std::string GlobalEvent::className = "GlobalEvent";
	}
}
