/*
 * BundleGeneratedEvent.h
 *
 *  Created on: 23.06.2010
 *      Author: morgenro
 */

#ifndef BUNDLEGENERATEDEVENT_H_
#define BUNDLEGENERATEDEVENT_H_

#include "core/Event.h"
#include "ibrdtn/data/Bundle.h"

namespace dtn
{
	namespace core
	{
		class BundleGeneratedEvent : public dtn::core::Event
		{
		public:
			virtual ~BundleGeneratedEvent();

			const string getName() const;

			string toString() const;

			static const string className;

			static void raise(const dtn::data::Bundle &bundle);

			const dtn::data::Bundle bundle;

		private:
			BundleGeneratedEvent(const dtn::data::Bundle &bundle);
		};
	}
}

#endif /* BUNDLEGENERATEDEVENT_H_ */
