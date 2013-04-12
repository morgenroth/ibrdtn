/*
 * NeighborDataset.h
 *
 *  Created on: 08.01.2013
 *      Author: morgenro
 */

#ifndef NEIGHBORDATASET_H_
#define NEIGHBORDATASET_H_

#include <ibrcommon/refcnt_ptr.h>
#include <stdlib.h>
#include <iostream>

namespace dtn
{
	namespace routing
	{
		class NeighborDataSetImpl {
		public:
			NeighborDataSetImpl(size_t id);
			virtual ~NeighborDataSetImpl() = 0;

			const size_t _dataset_id;
		};

		class NeighborDataset
		{
		private:
			class Empty : public NeighborDataSetImpl {
			public:
				Empty(size_t id) : NeighborDataSetImpl(id) { };
				virtual ~Empty() {};
			};

		public:
			NeighborDataset(size_t id);
			NeighborDataset(NeighborDataSetImpl *impl);
			~NeighborDataset();

			bool operator==(const NeighborDataset&) const;
			bool operator<(const NeighborDataset&) const;
			bool operator>(const NeighborDataset&) const;

			bool operator==(const size_t&) const;
			bool operator<(const size_t&) const;
			bool operator>(const size_t&) const;

			size_t getId() const;

			NeighborDataSetImpl& operator*();
			const NeighborDataSetImpl& operator*() const;

		private:
			refcnt_ptr<NeighborDataSetImpl> _impl;
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* NEIGHBORDATASET_H_ */
