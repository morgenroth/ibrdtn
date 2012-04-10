/*
 * Component.cpp
 *
 *  Created on: 21.04.2010
 *      Author: morgenro
 */

#include "Component.h"
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace daemon
	{
		Component::~Component()
		{
		}

		IndependentComponent::IndependentComponent()
		{
		}

		IndependentComponent::~IndependentComponent()
		{
			join();
		}

		void IndependentComponent::initialize()
		{
			componentUp();
		}

		void IndependentComponent::startup()
		{
			try {
				this->start();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER(error) << "failed to start IndependentComponent\n" << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void IndependentComponent::terminate()
		{
			componentDown();
			JoinableThread::stop();
		}

		void IndependentComponent::run()
		{
			componentRun();
		}

		IntegratedComponent::IntegratedComponent()
		{
		}

		IntegratedComponent::~IntegratedComponent()
		{
		}

		void IntegratedComponent::initialize()
		{
			componentUp();
		}

		void IntegratedComponent::startup()
		{
			// nothing to do
		}

		void IntegratedComponent::terminate()
		{
			componentDown();
		}
	}
}
