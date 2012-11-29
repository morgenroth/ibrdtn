/*
 * AES128Stream.h
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

#ifndef AES128STREAM_H_
#define AES128STREAM_H_

#include <streambuf>
#include <ostream>
#include <sys/types.h>
#include <stdint.h>
#include "ibrcommon/ssl/CipherStream.h"
#include "ibrcommon/ssl/gcm/gcm.h"

namespace ibrcommon
{
	/**
	Encrypts or decrypts an input stream using AES with a 128bit key using
	galois counter mode. In encryption mode initialisation vector and tag will
	be created and can be read with getIV() and getTag(). In decryption mode
	initialisation vector and tag have to be set at construction or via the
	decrypt()-Method.
	TODO test the gcm_iv structure on be and le systems
	*/
	class AES128Stream : public ibrcommon::CipherStream
	{
		public:
			/** the number of bytes of 128 bit */
			static const size_t key_size_in_bytes = 16;
			/** the number of bytes of the salt */
			static const size_t salt_len = sizeof(uint32_t);
			/** the number of bytes of the initialisation vector */
			static const size_t iv_len = 8;
			/** the number of bytes of the verification tag */
			static const size_t tag_len = 16;
			/** the size of the buffer in which the data will be streamed */
			static const size_t BUFF_SIZE = 2048;

			/**
			Creates a AES128Stream object, either for encrypting or decrypting,
			which is controlled by mode. If this object is used for decryption iv
			and tag have to be set.
			@param mode tell the constructor wether this will be used for en- or
			decryption
			@param output the stream in which will the cipher- or plaintext be
			serialized into
			@param key the AES128 key to use. Its size is key_size_in_bytes.
			@param salt the salt, which shall be the same for all data which belongs
			together
			@param iv if used for decryption, this is the initialisation vector,
			which was created at encryption. The size of this array is iv_len.
			@param tag if used for decryption, this is the authentication tag, which
			was created at encryption. The size of this array is tag_len.
			*/
			AES128Stream(const CipherMode mode, std::ostream& output, const unsigned char key[key_size_in_bytes], const uint32_t salt);
			AES128Stream(const CipherMode mode, std::ostream& output, const unsigned char key[key_size_in_bytes], const uint32_t salt, const unsigned char iv[iv_len]);

			/** cleans the output buffer and the context */
			virtual ~AES128Stream();

			/**
			Write the initialisation vector into an array, with length iv_len.
			@param to_iv the array in which the vector will be written into
			*/
			void getIV(unsigned char (&to_iv)[iv_len]) const;

			/**
			Write the authentication tag into an array, with length tag_len.
			@param to_tag the array in which the tag will be written into
			*/
			void getTag(unsigned char (&to_tag)[tag_len]);

			/**
			 * compares the given tag with the tag of the last en-/decryption
			 */
			bool verify(const unsigned char (&verify_tag)[tag_len]);

		protected:
			virtual void encrypt(char *buf, const size_t size);
			virtual void decrypt(char *buf, const size_t size);

		private:
			/** a way of putting salt and initialisation_vector together in memory,
			without the use of memcpy().\n
			TODO it needs to be tested if this behave on little and big endian
			equally*/
			typedef struct {
				uint32_t salt;
				unsigned char initialisation_vector[iv_len];
			} gcm_iv;

			/** the instance of the gcm_iv struct */
			gcm_iv _gcm_iv;

			/** the openssl context used for the AES operations */
			gcm_ctx _ctx;

			/**
			since the initilisation vector will be refilled with random bytes after encryption, a copy of the last one is stored here
			*/
			unsigned char _used_initialisation_vector[iv_len];
	};
}

#endif /* AES128STREAM_H_ */
