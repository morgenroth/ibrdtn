/*
 * SummaryVector.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "routing/SummaryVector.h"
#include <ibrcommon/Logger.h>
#include <ibrcommon/TimeMeasurement.h>

namespace dtn
{
	namespace routing
	{
		SummaryVector::SummaryVector(const std::set<dtn::data::MetaBundle> &list)
		 : _bf(8192, 2)
		{
			add(list);
		}

		SummaryVector::SummaryVector()
		 : _bf(8192, 2)
		{
		}

		SummaryVector::~SummaryVector()
		{
		}

		void SummaryVector::commit()
		{
			IBRCOMMON_LOGGER_DEBUG(60) << "rebuilt of the bloomfilter needed" << IBRCOMMON_LOGGER_ENDL;
			ibrcommon::TimeMeasurement tm;
			tm.start();

			_bf.clear();

			for (std::set<dtn::data::BundleID>::const_iterator iter = _ids.begin(); iter != _ids.end(); iter++)
			{
				_bf.insert( (*iter).toString() );
			}

			tm.stop();
			IBRCOMMON_LOGGER_DEBUG(60) << "rebuilt done in " << tm << IBRCOMMON_LOGGER_ENDL;
		}

		void SummaryVector::add(const std::set<dtn::data::MetaBundle> &list)
		{
			for (std::set<dtn::data::MetaBundle>::const_iterator iter = list.begin(); iter != list.end(); iter++)
			{
				add( *iter );
			}
		}

		bool SummaryVector::contains(const dtn::data::BundleID &id) const
		{
			return _bf.contains(id.toString());
		}

		void SummaryVector::add(const dtn::data::BundleID &id)
		{
			_bf.insert(id.toString());
			_ids.insert( id );
		}

		void SummaryVector::remove(const dtn::data::BundleID &id)
		{
			_ids.erase( id );
		}

		void SummaryVector::clear()
		{
			_bf.clear();
			_ids.clear();
		}

		const ibrcommon::BloomFilter& SummaryVector::getBloomFilter() const
		{
			return _bf;
		}

		std::set<dtn::data::BundleID> SummaryVector::getNotIn(ibrcommon::BloomFilter &filter) const
		{
			std::set<dtn::data::BundleID> ret;

//			// if the lists are equal return an empty list
//			if (filter == _bf) return ret;

			// iterate through all items to find the differences
			for (std::set<dtn::data::BundleID>::const_iterator iter = _ids.begin(); iter != _ids.end(); iter++)
			{
				if (!filter.contains( (*iter).toString() ) )
				{
					ret.insert( (*iter) );
				}
			}

			return ret;
		}

		size_t SummaryVector::getLength() const
		{
			return dtn::data::SDNV(_bf.size()).getLength() + _bf.size();
		}

		std::ostream &operator<<(std::ostream &stream, const SummaryVector &obj)
		{
			dtn::data::SDNV size(obj._bf.size());
			stream << size;

			const char *data = reinterpret_cast<const char*>(obj._bf.table());
			stream.write(data, obj._bf.size());

			return stream;
		}

		std::istream &operator>>(std::istream &stream, SummaryVector &obj)
		{
			dtn::data::SDNV count;
			stream >> count;

			char buffer[count.getValue()];

			stream.read(buffer, count.getValue());

			obj.clear();
			obj._bf.load((unsigned char*)buffer, count.getValue());

			return stream;
		}
	}
}
