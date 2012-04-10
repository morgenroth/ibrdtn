#ifndef BUNDLERECEIVER_H_
#define BUNDLERECEIVER_H_

#include "ibrdtn/data/Bundle.h"
#include "net/ConvergenceLayer.h"

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		class BundleReceiver
		{
			public:
			BundleReceiver() {};
			virtual ~BundleReceiver() {};
			virtual void received(const dtn::data::EID &eid, const Bundle &b) = 0;
		};
	}
}

#endif /*BUNDLERECEIVER_H_*/
