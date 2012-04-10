/*
 * BundleID.h
 *
 *  Created on: 01.09.2009
 *      Author: morgenro
 */

#ifndef BUNDLEID_H_
#define BUNDLEID_H_

#include "ibrdtn/data/EID.h"
#include "ibrdtn/data/Bundle.h"

namespace dtn
{
	namespace data
	{
		class BundleID
		{
		public:
			BundleID(const dtn::data::EID source = dtn::data::EID(), const size_t timestamp = 0, const size_t sequencenumber = 0, const bool fragment = false, const size_t offset = 0);
			BundleID(const dtn::data::Bundle &b);
			virtual ~BundleID();

			bool operator!=(const BundleID& other) const;
			bool operator==(const BundleID& other) const;
			bool operator<(const BundleID& other) const;
			bool operator>(const BundleID& other) const;

			string toString() const;
			size_t getTimestamp() const;

			friend std::ostream &operator<<(std::ostream &stream, const BundleID &obj);
			friend std::istream &operator>>(std::istream &stream, BundleID &obj);

			dtn::data::EID source;
			size_t timestamp;
			size_t sequencenumber;

			bool fragment;
			size_t offset;
		};
	}
}

#endif /* BUNDLEID_H_ */
