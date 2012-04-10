/*
 * GlobalEvent.h
 *
 *  Created on: 30.10.2009
 *      Author: morgenro
 */

#include "core/GlobalEvent.h"
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

		const string GlobalEvent::getName() const
		{
			return GlobalEvent::className;
		}

		void GlobalEvent::raise(const Action a)
		{
			// raise the new event
			raiseEvent( new GlobalEvent(a) );
		}

		string GlobalEvent::toString() const
		{
			return className;
		}

		const string GlobalEvent::className = "GlobalEvent";
	}
}
