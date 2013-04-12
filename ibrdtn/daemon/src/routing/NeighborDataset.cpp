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
		NeighborDataSetImpl::NeighborDataSetImpl(size_t id)
		: _dataset_id(id)
		{
		}

		NeighborDataSetImpl::~NeighborDataSetImpl()
		{
		}

		NeighborDataset::NeighborDataset(size_t id)
		 : _impl(new Empty(id))
		{
		}

		NeighborDataset::NeighborDataset(NeighborDataSetImpl *impl)
		 : _impl(impl)
		{ }

		NeighborDataset::~NeighborDataset()
		{ }

		bool NeighborDataset::operator==(const NeighborDataset &obj) const
		{
			return (obj.getId() == getId());
		}

		bool NeighborDataset::operator<(const NeighborDataset &obj) const
		{
			return (getId() < obj.getId());
		}

		bool NeighborDataset::operator>(const NeighborDataset &obj) const
		{
			return (getId() > obj.getId());
		}

		bool NeighborDataset::operator==(const size_t &obj) const
		{
			return (obj == getId());
		}

		bool NeighborDataset::operator<(const size_t &obj) const
		{
			return (getId() < obj);
		}

		bool NeighborDataset::operator>(const size_t &obj) const
		{
			return (getId() > obj);
		}

		size_t NeighborDataset::getId() const
		{
			return _impl->_dataset_id;
		}

		NeighborDataSetImpl& NeighborDataset::operator*()
		{
			return _impl.operator*();
		}

		const NeighborDataSetImpl& NeighborDataset::operator*() const
		{
			return _impl.operator*();
		}
	} /* namespace routing */
} /* namespace dtn */
