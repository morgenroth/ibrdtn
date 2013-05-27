/*
 * StrictSerializer.cpp
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

#include "ibrdtn/security/StrictSerializer.h"
#include "ibrdtn/security/BundleAuthenticationBlock.h"
#include "ibrdtn/security/PayloadIntegrityBlock.h"
#include "ibrdtn/data/SDNV.h"
#include "ibrdtn/data/Block.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/data/Dictionary.h"

namespace dtn
{
	namespace security
	{
		StrictSerializer::StrictSerializer(std::ostream& stream, const dtn::security::SecurityBlock::BLOCK_TYPES type, const bool with_correlator, const dtn::data::Number &correlator)
		 : DefaultSerializer(stream), _block_type(type), _with_correlator(with_correlator), _correlator(correlator)
		{
		}

		StrictSerializer::~StrictSerializer()
		{
		}

		dtn::data::Serializer& StrictSerializer::operator<<(const dtn::data::Bundle& bundle)
		{
			// rebuild the dictionary
			rebuildDictionary(bundle);

			// serialize the primary block
			(dtn::data::DefaultSerializer&)(*this) << static_cast<const dtn::data::PrimaryBlock&>(bundle);

			// serialize all secondary blocks
			dtn::data::Bundle::const_iterator iter = bundle.begin();

			// skip all blocks before the correlator
			for (; _with_correlator && iter != bundle.end(); ++iter)
			{
				const dtn::data::Block &b = (**iter);
				if (b.getType() == SecurityBlock::BUNDLE_AUTHENTICATION_BLOCK || b.getType() == SecurityBlock::PAYLOAD_INTEGRITY_BLOCK)
				{
					const dtn::security::SecurityBlock& sb = dynamic_cast<const dtn::security::SecurityBlock&>(**iter);
					if ((sb._ciphersuite_flags & SecurityBlock::CONTAINS_CORRELATOR) && sb._correlator == _correlator)
						break;
				}
			}

			// consume the first block. this block may have the same correlator set, 
			// we are searching for in the loop
			(*this) << (**iter);
			++iter;

			// serialize the remaining block
			for (; iter != bundle.end(); ++iter)
			{
				const dtn::data::Block &b = (**iter);
				(*this) << b;

				try {
					const dtn::security::SecurityBlock &sb = dynamic_cast<const dtn::security::SecurityBlock&>(b);

					if ( (sb.getType() == SecurityBlock::BUNDLE_AUTHENTICATION_BLOCK) || (sb.getType() == SecurityBlock::PAYLOAD_INTEGRITY_BLOCK) )
					{
						// until the block with the second correlator is reached
						if (_with_correlator && (sb._ciphersuite_flags & SecurityBlock::CONTAINS_CORRELATOR) && sb._correlator == _correlator) break;
					}
				} catch (const std::bad_cast&) { };
			}

			return *this;
		}

		dtn::data::Serializer& StrictSerializer::operator<<(const dtn::data::Block &obj)
		{
			_stream << obj.getType();
			_stream << obj.getProcessingFlags();

			const dtn::data::Block::eid_list &eids = obj.getEIDList();

#ifdef __DEVELOPMENT_ASSERTIONS__
			// test: BLOCK_CONTAINS_EIDS => (_eids.size() > 0)
			assert(!obj.get(dtn::data::Block::BLOCK_CONTAINS_EIDS) || (eids.size() > 0));
#endif

			if (obj.get(dtn::data::Block::BLOCK_CONTAINS_EIDS))
			{
				_stream << dtn::data::Number(eids.size());
				for (dtn::data::Block::eid_list::const_iterator it = eids.begin(); it != eids.end(); ++it)
				{
					dtn::data::Dictionary::Reference offsets;

					if (_compressable)
					{
						offsets = (*it).getCompressed();
					}
					else
					{
						offsets = _dictionary.getRef(*it);
					}

					_stream << offsets.first;
					_stream << offsets.second;
				}
			}

			// write size of the payload in the block
			_stream << dtn::data::Number(obj.getLength_strict());

			dtn::data::Length slength = 0;
			obj.serialize_strict(_stream, slength);

			return (*this);
		}
	}
}
