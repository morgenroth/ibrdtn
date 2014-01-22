/*
 * Bundle.h
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

#ifndef BUNDLE_H_
#define BUNDLE_H_

#include "ibrdtn/data/Number.h"
#include "ibrdtn/data/EID.h"
#include "ibrdtn/data/Block.h"
#include "ibrdtn/data/PrimaryBlock.h"
#include "ibrdtn/data/PayloadBlock.h"
#include "ibrdtn/data/ExtensionBlock.h"
#include <ibrcommon/refcnt_ptr.h>
#include <ibrcommon/Iterator.h>
#include <ostream>
#ifdef __DEVELOPMENT_ASSERTIONS__
#include <cassert>
#endif
#include <set>
#include <map>
#include <typeinfo>
#include <list>
#include <iterator>
#include <algorithm>

namespace dtn
{
	namespace data
	{
		class BundleBuilder;
		class MetaBundle;
		class BundleID;

		class Bundle : public PrimaryBlock
		{
			friend class BundleBuilder;

		public:
			class NoSuchBlockFoundException : public ibrcommon::Exception
			{
				public:
					NoSuchBlockFoundException() : ibrcommon::Exception("No block found with this Block ID.")
					{
					};
			};

			class block_elem : public refcnt_ptr<Block>
			{
			public:
				block_elem(dtn::data::Block *block) : refcnt_ptr<Block>(block) { }
				bool operator==(const dtn::data::block_t &type) const {
					return (**this) == type;
				}
			};

			typedef std::list<block_elem> block_list;

			typedef block_list::iterator iterator;
			typedef block_list::const_iterator const_iterator;

			typedef ibrcommon::find_iterator<iterator, block_t> find_iterator;
			typedef ibrcommon::find_iterator<const_iterator, block_t> const_find_iterator;

			iterator begin();
			iterator end();
			const_iterator begin() const;
			const_iterator end() const;

			iterator find(block_t blocktype);
			const_iterator find(block_t blocktype) const;

			iterator find(const Block &block);
			const_iterator find(const Block &block) const;

			Bundle(bool zero_timestamp = false);
			virtual ~Bundle();

			bool operator==(const BundleID& other) const;
			bool operator==(const MetaBundle& other) const;

			bool operator==(const Bundle& other) const;
			bool operator!=(const Bundle& other) const;
			bool operator<(const Bundle& other) const;
			bool operator>(const Bundle& other) const;

			template<class T>
			T& find();

			template<class T>
			const T& find() const;

			template<class T>
			T& push_front();

			template<class T>
			T& push_back();

			template<class T>
			T& insert(iterator before);

			dtn::data::PayloadBlock& push_front(ibrcommon::BLOB::Reference &ref);
			dtn::data::PayloadBlock& push_back(ibrcommon::BLOB::Reference &ref);
			dtn::data::PayloadBlock& insert(iterator before, ibrcommon::BLOB::Reference &ref);

			dtn::data::Block& push_front(ExtensionBlock::Factory &factory);
			dtn::data::Block& push_back(ExtensionBlock::Factory &factory);
			dtn::data::Block& insert(iterator before, ExtensionBlock::Factory &factory);

			void erase(iterator it);
			void erase(iterator begin, iterator end);

			void remove(const Block &block);

			void clear();

			Size size() const;

			bool allEIDsInCBHE() const;

			dtn::data::Length getPayloadLength() const;

		private:
			block_list _blocks;
		};

		template<class T>
		T& Bundle::find()
		{
			iterator it = this->find(T::BLOCK_TYPE);
			if (it == this->end()) throw NoSuchBlockFoundException();
			return dynamic_cast<T&>(**it);
		}

		template<class T>
		const T& Bundle::find() const
		{
			const_iterator it = this->find(T::BLOCK_TYPE);
			if (it == this->end()) throw NoSuchBlockFoundException();
			return dynamic_cast<const T&>(**it);
		}

		template<class T>
		T& Bundle::push_front()
		{
			T *tmpblock = new T();
			block_elem block( static_cast<dtn::data::Block*>(tmpblock) );
			_blocks.push_front(block);

			// if this was the first element
			if (size() == 1)
			{
				// set the last block bit
				iterator last = end();
				last--;
				(**last).set(dtn::data::Block::LAST_BLOCK, true);
			}

			return (*tmpblock);
		}

		template<class T>
		T& Bundle::push_back()
		{
			if (size() > 0) {
				// remove the last block bit
				iterator last = end();
				last--;
				(**last).set(dtn::data::Block::LAST_BLOCK, false);
			}

			T *tmpblock = new T();
			block_elem block( static_cast<dtn::data::Block*>(tmpblock) );
			_blocks.push_back(block);

			// set the last block bit
			block->set(dtn::data::Block::LAST_BLOCK, true);

			return (*tmpblock);
		}

		template<class T>
		T& Bundle::insert(iterator before)
		{
			if (size() > 0) {
				// remove the last block bit
				iterator last = end();
				last--;
				(**last).set(dtn::data::Block::LAST_BLOCK, false);
			}

			T *tmpblock = new T();
			block_elem block( static_cast<dtn::data::Block*>(tmpblock) );
			_blocks.insert(before, block);

			// set the last block bit
			iterator last = end();
			last--;
			(**last).set(dtn::data::Block::LAST_BLOCK, true);

			return (*tmpblock);
		}
	}
}

#endif /* BUNDLE_H_ */
