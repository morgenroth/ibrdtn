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
				StrictSerializer(std::ostream& stream, const SecurityBlock::BLOCK_TYPES type = SecurityBlock::BUNDLE_AUTHENTICATION_BLOCK, const bool with_correlator = false, const u_int64_t correlator = 0);

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
				const u_int64_t _correlator;
		};
	}
}

#endif
