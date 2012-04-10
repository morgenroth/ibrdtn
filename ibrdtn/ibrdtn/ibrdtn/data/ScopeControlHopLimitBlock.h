/*
 * ScopeControlHopLimitBlock.h
 *
 *  Created on: 05.08.2011
 *      Author: morgenro
 */

#include <ibrdtn/data/Block.h>
#include <ibrdtn/data/SDNV.h>
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

			static const char BLOCK_TYPE = 199;

			ScopeControlHopLimitBlock();
			virtual ~ScopeControlHopLimitBlock();

			size_t getHopsToLive() const;
			void increment(size_t hops = 1);
			void setLimit(size_t hops);

			virtual size_t getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, size_t &length) const;
			virtual std::istream &deserialize(std::istream &stream, const size_t length);

		private:
			dtn::data::SDNV _count;
			dtn::data::SDNV _limit;
		};

		/**
		 * This creates a static block factory
		 */
		static ScopeControlHopLimitBlock::Factory __ScopeControlHopLimitBlockFactory__;
	} /* namespace data */
} /* namespace dtn */
#endif /* SCOPECONTROLHOPLIMITBLOCK_H_ */
