/*
 * NeighborDataset.h
 *
 *  Created on: 08.01.2013
 *      Author: morgenro
 */

#ifndef NEIGHBORDATASET_H_
#define NEIGHBORDATASET_H_

#include <stdlib.h>
#include <iostream>

namespace dtn
{
	namespace routing
	{
		class NeighborDataset
		{
		public:
			NeighborDataset(size_t id);
			virtual ~NeighborDataset() = 0;

			bool operator==(const NeighborDataset&) const;
			bool operator<(const NeighborDataset&) const;

			const size_t id;
		};
	} /* namespace routing */
} /* namespace dtn */
#endif /* NEIGHBORDATASET_H_ */
