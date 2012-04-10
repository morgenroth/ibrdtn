/*
 * RequeueBundleEvent.h
 *
 *  Created on: 15.02.2010
 *      Author: morgenro
 */

#ifndef REQUEUEBUNDLEEVENT_H_
#define REQUEUEBUNDLEEVENT_H_

#include "core/Event.h"
#include "ibrdtn/data/BundleID.h"
#include "ibrdtn/data/EID.h"

namespace dtn
{
	namespace routing
	{
		class RequeueBundleEvent : public dtn::core::Event
		{
		public:
			virtual ~RequeueBundleEvent();

			const string getName() const;

			string toString() const;

			static const string className;

			static void raise(const dtn::data::EID peer, const dtn::data::BundleID &id);

			dtn::data::EID _peer;
			dtn::data::BundleID _bundle;

		private:
			RequeueBundleEvent(const dtn::data::EID peer, const dtn::data::BundleID &id);
		};
	}
}


#endif /* REQUEUEBUNDLEEVENT_H_ */
