/*
 * StreamBlock.h
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

#ifndef STREAMBLOCK_H_
#define STREAMBLOCK_H_

#include "ibrdtn/data/Number.h"
#include "ibrdtn/data/Block.h"
#include "ibrdtn/data/ExtensionBlock.h"

namespace dtn
{
	namespace data
	{
		class StreamBlock : public dtn::data::Block
		{
		public:
			class Factory : public dtn::data::ExtensionBlock::Factory
			{
			public:
				Factory() : dtn::data::ExtensionBlock::Factory(StreamBlock::BLOCK_TYPE) {};
				virtual ~Factory() {};
				virtual dtn::data::Block* create();
			};

			static const dtn::data::block_t BLOCK_TYPE;

			enum STREAM_FLAGS
			{
				STREAM_BEGIN = 1,
				STREAM_END = 1 << 0x01
			};

			StreamBlock();
			virtual ~StreamBlock();

			virtual Length getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, Length &length) const;
			virtual std::istream &deserialize(std::istream &stream, const Length &length);

			void setSequenceNumber(Number seq);
			const Number& getSequenceNumber() const;

			void set(STREAM_FLAGS flag, const bool &value);
			bool get(STREAM_FLAGS flag) const;

		private:
			Number _seq;
			Bitset<STREAM_FLAGS> _streamflags;
		};

		/**
		 * This creates a static block factory
		 */
		static StreamBlock::Factory __StreamBlockFactory__;
	} /* namespace data */
} /* namespace dtn */
#endif /* STREAMBLOCK_H_ */
