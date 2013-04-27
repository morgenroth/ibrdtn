/*
 * NeighborDataset.h
 *
 *  Created on: 08.01.2013
 *      Author: morgenro
 */

#ifndef NEIGHBORDATASET_H_
#define NEIGHBORDATASET_H_

#include <ibrdtn/data/SDNV.h>
#include <ibrcommon/refcnt_ptr.h>
#include <stdlib.h>
#include <iostream>

namespace dtn
{
	namespace routing
	{
		class NeighborDataSetImpl {
		public:
			NeighborDataSetImpl(const dtn::data::SDNV &id);
			virtual ~NeighborDataSetImpl() = 0;

			const dtn::data::SDNV _dataset_id;
		};

		class NeighborDataset
		{
		private:
			class Empty : public NeighborDataSetImpl {
			public:
				Empty(const dtn::data::SDNV &id) : NeighborDataSetImpl(id) { };
				virtual ~Empty() {};
			};

		public:
			NeighborDataset(const dtn::data::SDNV &id);
			NeighborDataset(NeighborDataSetImpl *impl);
			~NeighborDataset();

			bool operator==(const NeighborDataset&) const;
			bool operator<(const NeighborDataset&) const;
			bool operator>(const NeighborDataset&) const;

			bool operator==(const dtn::data::SDNV&) const;
			bool operator<(const dtn::data::SDNV&) const;
			bool operator>(const dtn::data::SDNV&) const;

			const dtn::data::SDNV &getId() const;

			NeighborDataSetImpl& operator*();
			const NeighborDataSetImpl& operator*() const;

		private:
			refcnt_ptr<NeighborDataSetImpl> _impl;
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* NEIGHBORDATASET_H_ */
