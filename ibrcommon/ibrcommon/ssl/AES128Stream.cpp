/*
 * AES128Stream.cpp
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

#include "ibrcommon/ssl/AES128Stream.h"
#include "ibrcommon/Logger.h"
#include <openssl/rand.h>
#include <netinet/in.h>

namespace ibrcommon
{
	AES128Stream::AES128Stream(const CipherMode mode, std::ostream& output, const unsigned char key[key_size_in_bytes], const uint32_t salt)
		: CipherStream(output, mode)
	{
		// init gcm and load the key into the context
		if (gcm_init_and_key(key, key_size_in_bytes, &_ctx))
			IBRCOMMON_LOGGER_TAG("AES128Stream", critical) << "failed to initialize aes gcm context" << IBRCOMMON_LOGGER_ENDL;

		// convert the salt to network byte order
		_gcm_iv.salt = htonl(salt);

		// generate a random IV
		if (!RAND_bytes(_gcm_iv.initialisation_vector, iv_len))
			IBRCOMMON_LOGGER_TAG("AES128Stream", critical) << "failed to create initialization vector" << IBRCOMMON_LOGGER_ENDL;

		// copy the IV to a local array
		for (unsigned int i = 0; i < iv_len; ++i)
			_used_initialisation_vector[i] = _gcm_iv.initialisation_vector[i];

		// init the GCM message
		gcm_init_message(reinterpret_cast<unsigned char *>(&_gcm_iv), sizeof(gcm_iv), &_ctx);
	}

	AES128Stream::AES128Stream(const CipherMode mode, std::ostream& output, const unsigned char key[key_size_in_bytes], const uint32_t salt, const unsigned char iv[iv_len])
		: CipherStream(output, mode)
	{
		// init gcm and load the key into the context
		if (gcm_init_and_key(key, key_size_in_bytes, &_ctx))
			IBRCOMMON_LOGGER_TAG("AES128Stream", critical) << "failed to initialize aes gcm context" << IBRCOMMON_LOGGER_ENDL;

		// convert the salt to network byte order
		_gcm_iv.salt = htonl(salt);

		// copy the IV to local variables
		for (unsigned int i = 0; i < iv_len; ++i)
		{
			_gcm_iv.initialisation_vector[i] = iv[i];
			_used_initialisation_vector[i] = iv[i];
		}

		// init the GCM message
		gcm_init_message(reinterpret_cast<unsigned char *>(&_gcm_iv), sizeof(gcm_iv), &_ctx);
	}

	AES128Stream::~AES128Stream()
	{
		// close the GCM context
		gcm_end(&_ctx);
	}

	void AES128Stream::getIV(unsigned char (&to_iv)[iv_len]) const
	{
		for (unsigned int i = 0; i < iv_len; ++i)
			to_iv[i] = _used_initialisation_vector[i];
	}

	void AES128Stream::getTag(unsigned char (&to_tag)[tag_len])
	{
		ret_type rr = gcm_compute_tag((unsigned char*)to_tag, tag_len, &_ctx);

		if (rr != RETURN_OK)
			throw ibrcommon::Exception("tag generation failed");
	}

	bool AES128Stream::verify(const unsigned char (&verify_tag)[tag_len])
	{
		try {
			// compute the current tag
			unsigned char tag[tag_len]; getTag(tag);

			// compare the tags
			return (::memcmp(tag, verify_tag, tag_len) == 0);
		} catch (const ibrcommon::Exception&) {
			return false;
		}
	}

	void AES128Stream::encrypt(char *buf, const size_t size)
	{
		gcm_encrypt(reinterpret_cast<unsigned char *>(buf), size, &_ctx);
	}

	void AES128Stream::decrypt(char *buf, const size_t size)
	{
		gcm_decrypt(reinterpret_cast<unsigned char *>(buf), size, &_ctx);
	}
}
