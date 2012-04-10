/*
 * CompressedPayloadBlock.h
 *
 *  Created on: 20.04.2011
 *      Author: morgenro
 */

#include <ibrdtn/data/Block.h>
#include <ibrdtn/data/SDNV.h>
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

			static const char BLOCK_TYPE = 202;

			enum COMPRESS_ALGS
			{
				COMPRESSION_UNKNOWN = 0,
				COMPRESSION_ZLIB = 1,
				COMPRESSION_BZ2 = 2
			};

			CompressedPayloadBlock();
			virtual ~CompressedPayloadBlock();

			virtual size_t getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, size_t &length) const;
			virtual std::istream &deserialize(std::istream &stream, const size_t length);

			void setAlgorithm(COMPRESS_ALGS alg);
			COMPRESS_ALGS getAlgorithm() const;

			void setOriginSize(size_t s);
			size_t getOriginSize() const;

			static void compress(dtn::data::Bundle &b, COMPRESS_ALGS alg);
			static void extract(dtn::data::Bundle &b);

		private:
			static void compress(CompressedPayloadBlock::COMPRESS_ALGS alg, std::istream &is, std::ostream &os);
			static void extract(CompressedPayloadBlock::COMPRESS_ALGS alg, std::istream &is, std::ostream &os);

			dtn::data::SDNV _algorithm;
			dtn::data::SDNV _origin_size;
		};

		/**
		 * This creates a static block factory
		 */
		static CompressedPayloadBlock::Factory __CompressedPayloadBlockFactory__;
	}
}

#endif /* COMPRESSEDPAYLOADBLOCK_H_ */
