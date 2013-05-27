/*
 * StrictSerializer.h
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

#ifndef __STRICT_SERIALIZER_H__
#define __STRICT_SERIALIZER_H__

#include "ibrdtn/data/Serializer.h"
#include "ibrdtn/security/SecurityBlock.h"

namespace dtn
{
	namespace security
	{
		/**
		Serializes a bundle in strict canonical form in order to hash it for 
		BundleAuthenticationBlocks. Use serialize_strict() to get the strict 
		canonical form written into a stream. Its behaviour differs not much from 
		the DefaultSerializer. The only exception is the special treatment of the 
		BundleAuthenticationBlock or PayloadIntegrityBlock. Their security result 
		will not be serialized.
		*/
		class StrictSerializer : public dtn::data::DefaultSerializer
		{
			public:
				/**
				Constructs a StrictSerializer, which will stream into stream
				*/
				StrictSerializer(std::ostream& stream, const SecurityBlock::BLOCK_TYPES type = SecurityBlock::BUNDLE_AUTHENTICATION_BLOCK, const bool with_correlator = false, const dtn::data::Number &correlator = 0);

				/** does nothing */
				virtual ~StrictSerializer();

				/**
				Serializes the given bundle and ignores the security result of the block
				type. If a correlator is given only the primary block and the blocks
				containing this correlator and the date between them will be written.
				@param bundle the bundle to canonicalized
				@param type type of block, which security result shall be ignored
				@param with_correlator says if correlator is used
				@param correlator the used correlator
				@return a reference to this instance
				*/
				virtual dtn::data::Serializer &operator<<(const dtn::data::Bundle &obj);

				virtual dtn::data::Serializer &operator<<(const dtn::data::Block &obj);

			private:
				const dtn::security::SecurityBlock::BLOCK_TYPES _block_type;
				const bool _with_correlator;
				const dtn::data::Number _correlator;
		};
	}
}

#endif
