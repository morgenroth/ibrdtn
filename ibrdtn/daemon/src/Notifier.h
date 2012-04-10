/*
 * Notifier.h
 *
 *  Created on: 11.12.2009
 *      Author: morgenro
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
