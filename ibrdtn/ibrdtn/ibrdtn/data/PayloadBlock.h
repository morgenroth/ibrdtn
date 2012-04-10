/*
 * PayloadBlock.h
 *
 *  Created on: 29.05.2009
 *      Author: morgenro
 */



#ifndef PAYLOADBLOCK_H_
#define PAYLOADBLOCK_H_

#include "ibrdtn/data/Block.h"
#include "ibrcommon/data/BLOB.h"

namespace dtn
{
	namespace data
	{
		class PayloadBlock : public Block
		{
		public:
			static const char BLOCK_TYPE = 1;

			PayloadBlock();
			PayloadBlock(ibrcommon::BLOB::Reference ref);
			virtual ~PayloadBlock();

			ibrcommon::BLOB::Reference getBLOB() const;

			virtual size_t getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, size_t &length) const;
			virtual std::istream &deserialize(std::istream &stream, const size_t length);

			/**
			 * serialize only a part of the payload
			 * @param stream The stream to serialize to.
			 * @param clip_offset The data offset of the payload.
			 * @param clip_length The length of the data.
			 * @return The previously given stream.
			 */
			std::ostream &serialize(std::ostream &stream, size_t clip_offset, size_t clip_length) const;

		private:
			ibrcommon::BLOB::Reference _blobref;
		};
	}
}

#endif /* PAYLOADBLOCK_H_ */
