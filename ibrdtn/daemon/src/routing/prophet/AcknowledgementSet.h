/*
 * AcknowledgementSet.h
 *
 *  Created on: 11.01.2013
 *      Author: morgenro
 */

#ifndef ACKNOWLEDGEMENTSET_H_
#define ACKNOWLEDGEMENTSET_H_

#include "routing/NodeHandshake.h"
#include <ibrdtn/data/BundleList.h>
#include <ibrcommon/thread/Mutex.h>
#include <iostream>
#include <set>

namespace dtn
{
	namespace routing
	{
		/*!
		 * \brief Set of Acknowledgements, that can be serialized in node handshakes.
		 */
		class AcknowledgementSet : public NodeHandshakeItem, public ibrcommon::Mutex, public dtn::data::BundleList
		{
		public:
			AcknowledgementSet();
			AcknowledgementSet(const AcknowledgementSet&);

			void merge(const AcknowledgementSet&); ///< merge the set with a second AcknowledgementSet
			bool has(const dtn::data::BundleID &bundle) const; ///< check if an acknowledgement exists for the bundle

			/* virtual methods from NodeHandshakeItem */
			virtual size_t getIdentifier() const; ///< \see NodeHandshakeItem::getIdentifier
			virtual size_t getLength() const; ///< \see NodeHandshakeItem::getLength
			virtual std::ostream& serialize(std::ostream& stream) const; ///< \see NodeHandshakeItem::serialize
			virtual std::istream& deserialize(std::istream& stream); ///< \see NodeHandshakeItem::deserialize
			static const size_t identifier;

			friend std::ostream& operator<<(std::ostream&, const AcknowledgementSet&);
			friend std::istream& operator>>(std::istream&, AcknowledgementSet&);
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* ACKNOWLEDGEMENTSET_H_ */
