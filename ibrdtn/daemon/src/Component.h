/*
 * Component.h
 *
 *  Created on: 21.04.2010
 *      Author: morgenro
 */

#ifndef COMPONENT_H_
#define COMPONENT_H_

#include <ibrcommon/thread/Thread.h>
#include <ibrcommon/thread/Mutex.h>

namespace dtn
{
	namespace daemon
	{
		class Component
		{
		public:
			/**
			 * Destructor of the component.
			 * This should be called after all components are terminated.
			 * @return
			 */
			virtual ~Component() = 0;

			/**
			 * Set up the component.
			 * At this stage no other components should be used.
			 */
			virtual void initialize() = 0;

			/**
			 * Start up the component.
			 * At this stage all other components are ready.
			 */
			virtual void startup() = 0;

			/**
			 * Terminate the component and do some cleanup stuff.
			 * All other components still exists, but may not serve signals.
			 */
			virtual void terminate() = 0;

			/**
			 * Return an identifier for this component
			 * @return
			 */
			virtual const std::string getName() const = 0;
		};

		/**
		 * Independent components are working in an own thread.
		 */
		class IndependentComponent : public Component, protected ibrcommon::JoinableThread
		{
		public:
			IndependentComponent();
			virtual ~IndependentComponent();

			virtual void initialize();
			virtual void startup();
			virtual void terminate();

		protected:
			void run();
			virtual void __cancellation() = 0;

			virtual void componentUp() = 0;
			virtual void componentRun() = 0;
			virtual void componentDown() = 0;
		};

		class IntegratedComponent : public Component
		{
		public:
			IntegratedComponent();
			virtual ~IntegratedComponent();

			virtual void initialize();
			virtual void startup();
			virtual void terminate();

		protected:
			virtual void componentUp() = 0;
			virtual void componentDown() = 0;
		};
	}
}

#endif /* COMPONENT_H_ */
