/*
 * Notifier.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef NOTIFIER_H_
#define NOTIFIER_H_

#include "Component.h"
#include "core/EventReceiver.h"
#include <iostream>

namespace dtn
{
	namespace daemon
	{
		class Notifier : public dtn::core::EventReceiver, public IntegratedComponent
		{
		public:
			Notifier(std::string cmd);
			virtual ~Notifier();

			void raiseEvent(const dtn::core::Event *evt);

			void notify(std::string title, std::string msg);

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;

		protected:
			virtual void componentUp();
			virtual void componentDown();

		private:
			std::string _cmd;
		};
	}
}

#endif /* NOTIFIER_H_ */
