/*
 * MetaBundle.h
 *
 *  Created on: 16.02.2010
 *      Author: morgenro
 */

#ifndef METABUNDLE_H_
#define METABUNDLE_H_

#include "ibrdtn/data/BundleID.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/DTNTime.h"
#include "ibrdtn/data/EID.h"

namespace dtn
{
	namespace data
	{
		class MetaBundle : public dtn::data::BundleID
		{
		public:
			MetaBundle();
			MetaBundle(const dtn::data::BundleID &id);
			MetaBundle(const dtn::data::Bundle &b);
			virtual ~MetaBundle();

			int getPriority() const;
			bool get(dtn::data::PrimaryBlock::FLAGS flag) const;

			dtn::data::DTNTime received;
			size_t lifetime;
			dtn::data::EID destination;
			dtn::data::EID reportto;
			dtn::data::EID custodian;
			size_t appdatalength;
			size_t procflags;
			size_t expiretime;
			size_t hopcount;
			size_t payloadlength;
		};
	}
}

#endif /* METABUNDLE_H_ */
