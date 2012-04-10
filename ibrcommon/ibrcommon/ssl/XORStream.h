/*
 * XORStream.h
 *
 *  Created on: 13.07.2010
 *      Author: morgenro
 */

#ifndef XORSTREAM_H_
#define XORSTREAM_H_

#include "ibrcommon/ssl/CipherStream.h"
#include <string>

namespace ibrcommon
{
	class XORStream : public CipherStream
	{
	public:
		XORStream(std::ostream &stream, const CipherMode mode, std::string key);
		virtual ~XORStream();

	protected:
		virtual void encrypt(char *buf, const size_t size);
		virtual void decrypt(char *buf, const size_t size);

	private:
		std::string _key;
		size_t _key_pos;
	};
}

#endif /* XORSTREAM_H_ */
