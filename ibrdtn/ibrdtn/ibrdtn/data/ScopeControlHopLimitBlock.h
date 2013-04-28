/*
 * ScopeControlHopLimitBlock.h
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

#include <ibrdtn/data/Block.h>
#include <ibrdtn/data/Number.h>
#include <ibrdtn/data/ExtensionBlock.h>

#ifndef SCOPECONTROLHOPLIMITBLOCK_H_
#define SCOPECONTROLHOPLIMITBLOCK_H_

namespace dtn
{
	namespace data
	{
		class ScopeControlHopLimitBlock : public dtn::data::Block
		{
		public:
			class Factory : public dtn::data::ExtensionBlock::Factory
			{
			public:
				Factory() : dtn::data::ExtensionBlock::Factory(ScopeControlHopLimitBlock::BLOCK_TYPE) {};
				virtual ~Factory() {};
				virtual dtn::data::Block* create();
			};

			static const dtn::data::block_t BLOCK_TYPE;

			ScopeControlHopLimitBlock();
			virtual ~ScopeControlHopLimitBlock();

			Number getHopsToLive() const;
			void increment(const Number &hops = 1);
			void setLimit(const Number &hops);

			virtual Length getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, Length &length) const;
			virtual std::istream &deserialize(std::istream &stream, const Length &length);

		private:
			dtn::data::Number _count;
			dtn::data::Number _limit;
		};

		/**
		 * This creates a static block factory
		 */
		static ScopeControlHopLimitBlock::Factory __ScopeControlHopLimitBlockFactory__;
	} /* namespace data */
} /* namespace dtn */
#endif /* SCOPECONTROLHOPLIMITBLOCK_H_ */
