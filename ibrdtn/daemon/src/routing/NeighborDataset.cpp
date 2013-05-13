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
		NeighborDataSetImpl::NeighborDataSetImpl(const dtn::data::Number &id)
		: _dataset_id(id)
		{
		}

		NeighborDataSetImpl::~NeighborDataSetImpl()
		{
		}

		NeighborDataset::NeighborDataset(const dtn::data::Number &id)
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

		bool NeighborDataset::operator==(const dtn::data::Number &obj) const
		{
			return (obj == getId());
		}

		bool NeighborDataset::operator<(const dtn::data::Number &obj) const
		{
			return (getId() < obj);
		}

		bool NeighborDataset::operator>(const dtn::data::Number &obj) const
		{
			return (getId() > obj);
		}

		const dtn::data::Number& NeighborDataset::getId() const
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
