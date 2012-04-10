/*
 * HMacStream.h
 *
 *  Created on: 22.06.2010
 *      Author: morgenro
 */

#ifndef HMACSTREAM_H_
#define HMACSTREAM_H_

#include "ibrcommon/ssl/HashStream.h"
#include <openssl/hmac.h>

namespace ibrcommon
{
	class HMacStream : public ibrcommon::HashStream
	{
	public:
		// The size of the input and output buffers.
		static const size_t BUFF_SIZE = 2048;

		HMacStream(const unsigned char * const key, const size_t key_size);
		virtual ~HMacStream();

	protected:
		virtual void update(char *buf, const size_t size);
		virtual void finalize(char * hash, unsigned int &size);

	private:
		const unsigned char * const key_;
		const size_t key_size_;

		HMAC_CTX ctx_;
	};
}

#endif /* HMACSTREAM_H_ */
