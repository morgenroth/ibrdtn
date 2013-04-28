/*
 * NeighborDataset.h
 *
 *  Created on: 08.01.2013
 *      Author: morgenro
 */

#ifndef NEIGHBORDATASET_H_
#define NEIGHBORDATASET_H_

#include <ibrdtn/data/Number.h>
#include <ibrcommon/refcnt_ptr.h>
#include <stdlib.h>
#include <iostream>

namespace dtn
{
	namespace routing
	{
		class NeighborDataSetImpl {
		public:
			NeighborDataSetImpl(const dtn::data::Number &id);
			virtual ~NeighborDataSetImpl() = 0;

			const dtn::data::Number _dataset_id;
		};

		class NeighborDataset
		{
		private:
			class Empty : public NeighborDataSetImpl {
			public:
				Empty(const dtn::data::Number &id) : NeighborDataSetImpl(id) { };
				virtual ~Empty() {};
			};

		public:
			NeighborDataset(const dtn::data::Number &id);
			NeighborDataset(NeighborDataSetImpl *impl);
			~NeighborDataset();

			bool operator==(const NeighborDataset&) const;
			bool operator<(const NeighborDataset&) const;
			bool operator>(const NeighborDataset&) const;

			bool operator==(const dtn::data::Number&) const;
			bool operator<(const dtn::data::Number&) const;
			bool operator>(const dtn::data::Number&) const;

			const dtn::data::Number &getId() const;

			NeighborDataSetImpl& operator*();
			const NeighborDataSetImpl& operator*() const;

		private:
			refcnt_ptr<NeighborDataSetImpl> _impl;
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* NEIGHBORDATASET_H_ */
