/*
 * MutableSerializer.h
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

#ifndef __MUTABLE_SERIALIZER_H__
#define __MUTABLE_SERIALIZER_H__

#include "ibrdtn/data/Serializer.h"
#include "ibrdtn/security/SecurityBlock.h"
#include "ibrdtn/data/Number.h"
#include <sys/types.h>

namespace dtn
{
	namespace security
	{
		class SecurityBlock;

		/**
		Serializes a bundle in mutable canonical form into a given stream. In 
		mutable canonical form all SDNVs are unpacked to 8 byte values and all EID 
		references are filled with the EID they point to. Every number is written in
		network byte order. Only payload related blocks are serialized. These are 
		the payload block, the PayloadIntegrityBlock and the 
		PayloadConfidentialBlock.
		*/
		class MutableSerializer : public dtn::data::DefaultSerializer
		{
			public:
				/**
				The size in bytes of a SDNV in mutable form in the stream
				*/
				static const dtn::data::Length sdnv_size = 8;
				
				/**
				Creates a MutableSerializer which will stream into stream
				@param stream the stream in which the mutable canonical form will be 
				written into
				*/
				MutableSerializer(std::ostream& stream, const dtn::data::Block *ignore = NULL);
				
				/** does nothing */
				virtual ~MutableSerializer();
				
				/**
				Serializes the primary block in mutable canonical form. The usual rules 
				for mutable canonicalisation (network byte order, unpacked SDNV, full 
				EIDs instead of references) apply here and some bits of the flags are 
				set to zero during this operation.
				@return a reference to this instance
				*/
				virtual Serializer &operator<<(const dtn::data::PrimaryBlock &obj);
				
				/**
				Serializes the block in mutable canonical form. The usual rules 
				for mutable canonicalisation (network byte order, unpacked SDNV, full 
				EIDs instead of references) apply here and some bits of the flags are 
				set to zero during this operation.
				@return a reference to this instance
				*/
				virtual Serializer &operator<<(const dtn::data::Block &obj);
				
				/**
				Not implemented. This is only required by the interface.
				*/
				virtual dtn::data::Length getLength(const dtn::data::Bundle &obj);
				
				/**
				Returns the length of the primary block in mutable canonical form.
				@param obj the primary block, of which the length shall be calculated
				@return the length of the primary block
				*/
				virtual dtn::data::Length getLength(const dtn::data::PrimaryBlock &obj) const;
				
				/**
				Returns the length of the block in mutable canonical form.
				@param obj the block, of which the length shall be calculated
				@return the length of the block
				*/
				virtual dtn::data::Length getLength(const dtn::data::Block &obj) const;

				/**
				Writes a uint32_t into stream in network byte order.
				@param stream the stream in which shall be written
				@param value the value to be written in network byte order
				@return the stream in which shall be written
				*/
				virtual Serializer &operator<<(const uint32_t value);
				
				/**
				Writes an EID to a stream in mutable form.
				@param stream the stream in which shall be written
				@param value the EID which shall be written
				@return the stream in which shall be written
				*/
				virtual Serializer &operator<<(const dtn::data::EID& value);
				
				/**
				Writes a SDNV to a stream in mutable form.
				@param stream the stream in which shall be written
				@param value the SDNV which shall be written
				@return the stream in which shall be written
				*/
				virtual Serializer &operator<<(const dtn::data::Number& value);

				/**
				 * Serialize a list of type-length-value entries.
				 * @param list
				 * @return
				 */
				virtual Serializer &operator<<(const dtn::security::SecurityBlock::TLVList& list);

			private:
				const dtn::data::Block *_ignore;
				bool _ignore_previous_bundles;
		};
	}
}

#endif
