/*
 * NeighborDataset.cpp
 *
 *  Created on: 08.01.2013
 *      Author: morgenro
 */

#include "routing/NeighborDataset.h"

namespace dtn
{
	namespace routing
	{
		NeighborDataset::NeighborDataset(size_t i)
		 : id(i)
		{ }

		NeighborDataset::~NeighborDataset()
		{ }

		bool NeighborDataset::operator==(const NeighborDataset &obj) const
		{
			return (obj.id == id);
		}

		bool NeighborDataset::operator<(const NeighborDataset &obj) const
		{
			return (id < obj.id);
		}
	} /* namespace routing */
} /* namespace dtn */
