/*
 * HMacStream.cpp
 *
 *  Created on: 22.06.2010
 *      Author: morgenro
 */

#include "ibrcommon/ssl/HMacStream.h"

namespace ibrcommon
{
	HMacStream::HMacStream(const unsigned char * const key, const size_t key_size)
	 : HashStream(BUFF_SIZE, EVP_MAX_MD_SIZE), key_(key), key_size_(key_size)
	{
		HMAC_CTX_init(&ctx_);
		HMAC_Init_ex(&ctx_, key_, key_size_, EVP_sha1(), NULL);
	}

	HMacStream::~HMacStream()
	{
		HMAC_CTX_cleanup(&ctx_);
	}

	void HMacStream::update(char *buf, const size_t size)
	{
		// hashing
		HMAC_Update(&ctx_, (unsigned char*)buf, size);
	}

	void HMacStream::finalize(char * hash, unsigned int &size)
	{
		HMAC_Final(&ctx_, (unsigned char*)hash, &size);
	}
}
