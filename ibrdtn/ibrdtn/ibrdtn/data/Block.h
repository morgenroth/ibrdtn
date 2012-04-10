/*
 * Block.h
 *
 *  Created on: 29.05.2009
 *      Author: morgenro
 */

#ifndef BLOCK_H_
#define BLOCK_H_

#include "ibrdtn/data/EID.h"
#include "ibrdtn/data/Exceptions.h"
#include "ibrdtn/data/SDNV.h"
#include "ibrdtn/data/Dictionary.h"
#include "ibrdtn/data/Serializer.h"
#include <ibrcommon/Exceptions.h>
#include <list>

namespace dtn
{
	namespace security
	{
		class StrictSerializer;
		class MutualSerializer;
	}

	namespace data
	{
		class Bundle;

		class Block
		{
			friend class Bundle;
			friend class DefaultSerializer;
			friend class dtn::security::StrictSerializer;
			friend class dtn::security::MutualSerializer;
			friend class DefaultDeserializer;
			friend class SeparateSerializer;
			friend class SeparateDeserializer;

		public:
			enum ProcFlags
			{
				REPLICATE_IN_EVERY_FRAGMENT = 1, 			// 0 - Block must be replicated in every fragment.
				TRANSMIT_STATUSREPORT_IF_NOT_PROCESSED = 1 << 0x01, // 1 - Transmit status report if block can't be processed.
				DELETE_BUNDLE_IF_NOT_PROCESSED = 1 << 0x02, 		// 2 - Delete bundle if block can't be processed.
				LAST_BLOCK = 1 << 0x03,								// 3 - Last block.
				DISCARD_IF_NOT_PROCESSED = 1 << 0x04,				// 4 - Discard block if it can't be processed.
				FORWARDED_WITHOUT_PROCESSED = 1 << 0x05,			// 5 - Block was forwarded without being processed.
				BLOCK_CONTAINS_EIDS = 1 << 0x06						// 6 - Block contains an EID-reference field.
			};

			virtual ~Block();

			virtual void addEID(const dtn::data::EID &eid);
			virtual std::list<dtn::data::EID> getEIDList() const;

			char getType() const { return _blocktype; }

			void set(ProcFlags flag, const bool &value);
			bool get(ProcFlags flag) const;

			/**
			 * Serialize the derived block payload.
			 * @param stream A output stream to serialize into.
			 * @return The same reference as given with the stream parameter.
			 */
			virtual std::ostream &serialize(std::ostream &stream, size_t &length) const = 0;

			/**
			 * Deserialize the derived block payload.
			 * @param stream A input stream to deserialize from.
			 * @return The same reference as given with the stream parameter.
			 */
			virtual std::istream &deserialize(std::istream &stream, const size_t length) = 0;

			/**
			 * Return the length of the payload, if this were an abstract block. It is
			 * the length put in the third field, after block type and processing flags.
			 */
			virtual size_t getLength() const = 0;

		protected:
			/**
			 * The constructor of this class is protected to prevent instantiation of this abstract class.
			 * @param blocktype The type of the block.
			 */
			Block(char blocktype);

			/**
			 * Return the length of the payload in strict format
			 */
			virtual size_t getLength_strict() const;

			/**
			Serialize the block in a strict way. Dynamic fields are set to the last deserialized value.
			@param stream the stream to be written into
			@return the same stream as the parameter for chaining
			*/
			virtual std::ostream &serialize_strict(std::ostream &stream, size_t &length) const;

			// block type of this block
			char _blocktype;

			// the list of EID references embedded in this block
			std::list<dtn::data::EID> _eids;

		private:
			// block processing flags
			size_t _procflags;
		};
	}
}

#endif /* BLOCK_H_ */
