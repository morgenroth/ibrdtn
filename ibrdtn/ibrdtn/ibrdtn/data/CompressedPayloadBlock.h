/*
 * CompressedPayloadBlock.h
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
#include "ibrdtn/data/Bundle.h"

#ifndef COMPRESSEDPAYLOADBLOCK_H_
#define COMPRESSEDPAYLOADBLOCK_H_

namespace dtn
{
	namespace data
	{
		class CompressedPayloadBlock : public dtn::data::Block
		{
		public:
			class Factory : public dtn::data::ExtensionBlock::Factory
			{
			public:
				Factory() : dtn::data::ExtensionBlock::Factory(CompressedPayloadBlock::BLOCK_TYPE) {};
				virtual ~Factory() {};
				virtual dtn::data::Block* create();
			};

			static const dtn::data::block_t BLOCK_TYPE;

			enum COMPRESS_ALGS
			{
				COMPRESSION_UNKNOWN = 0,
				COMPRESSION_ZLIB = 1,
				COMPRESSION_BZ2 = 2
			};

			CompressedPayloadBlock();
			virtual ~CompressedPayloadBlock();

			virtual Length getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, Length &length) const;
			virtual std::istream &deserialize(std::istream &stream, const Length &length);

			void setAlgorithm(COMPRESS_ALGS alg);
			COMPRESS_ALGS getAlgorithm() const;

			void setOriginSize(const Number &s);
			const Number& getOriginSize() const;

			static void compress(dtn::data::Bundle &b, COMPRESS_ALGS alg);
			static void extract(dtn::data::Bundle &b);

		private:
			static void compress(CompressedPayloadBlock::COMPRESS_ALGS alg, std::istream &is, std::ostream &os);
			static void extract(CompressedPayloadBlock::COMPRESS_ALGS alg, std::istream &is, std::ostream &os);

			dtn::data::Number _algorithm;
			dtn::data::Number _origin_size;
		};

		/**
		 * This creates a static block factory
		 */
		static CompressedPayloadBlock::Factory __CompressedPayloadBlockFactory__;
	}
}

#endif /* COMPRESSEDPAYLOADBLOCK_H_ */
