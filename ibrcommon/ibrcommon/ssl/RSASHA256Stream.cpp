/*
 * RSASHA256Stream.cpp
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

#include "ibrcommon/ssl/RSASHA256Stream.h"
#include "ibrcommon/Logger.h"
#include <openssl/err.h>

namespace ibrcommon
{
	RSASHA256Stream::RSASHA256Stream(EVP_PKEY * const pkey, bool verify)
	 : ostream(this), out_buf_(new char[BUFF_SIZE]), _pkey(pkey), _verify(verify), _sign_valid(false)
	{
		// Initialize get pointer.  This should be zero so that underflow is called upon first read.
		setp(out_buf_, out_buf_ + BUFF_SIZE - 1);
		EVP_MD_CTX_init(&_ctx);

		if (!_verify)
		{
			if (!EVP_SignInit_ex(&_ctx, EVP_sha256(), NULL))
			{
				IBRCOMMON_LOGGER(critical) << "failed to initialize the signature function" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
			}
		}
		else
		{
			if (!EVP_VerifyInit_ex(&_ctx, EVP_sha256(), NULL))
			{
				IBRCOMMON_LOGGER(critical) << "failed to initialize the verification function" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
			}
		}
	}

	RSASHA256Stream::~RSASHA256Stream()
	{
		EVP_MD_CTX_cleanup(&_ctx);
		delete[] out_buf_;
	}

	void RSASHA256Stream::reset()
	{
		EVP_MD_CTX_cleanup(&_ctx);

		EVP_MD_CTX_init(&_ctx);

		if (!_verify)
		{
			if (!EVP_SignInit_ex(&_ctx, EVP_sha256(), NULL))
			{
				IBRCOMMON_LOGGER(critical) << "failed to initialize the signature function" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
			}
		}
		else
		{
			if (!EVP_VerifyInit_ex(&_ctx, EVP_sha256(), NULL))
			{
				IBRCOMMON_LOGGER(critical) << "failed to initialize the verfication function" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
			}
		}

		_sign_valid = false;
	}

	const std::pair< const int, const std::string > RSASHA256Stream::getSign()
	{
		// check if data was feed into the stream and the sign needs to be
		// recalculated
		if (!_sign_valid)
		{
			sync();
			unsigned char * sign = new unsigned char[EVP_PKEY_size(_pkey)];
			unsigned int size;

			_return_code = EVP_SignFinal(&_ctx, sign, &size, _pkey);

			_sign = std::string(reinterpret_cast<const char * const>(sign), size);
			delete [] sign;
			_sign_valid = true;
		}
		return std::pair<const int, const std::string>(_return_code, _sign);
	}

	int RSASHA256Stream::getVerification(const std::string& their_sign)
	{
		// check if data was feed into the stream and the sign needs to be
		// recalculated
		if (!_sign_valid)
		{
			sync();
			_return_code = EVP_VerifyFinal(&_ctx, reinterpret_cast<const unsigned char *>(their_sign.c_str()), their_sign.size(), _pkey);
			_sign_valid = true;
		}
		return _return_code;
	}

	int RSASHA256Stream::sync()
	{
		int ret = std::char_traits<char>::eq_int_type(this->overflow(
				std::char_traits<char>::eof()), std::char_traits<char>::eof()) ? -1
				: 0;

		return ret;
	}

	RSASHA256Stream::traits::int_type RSASHA256Stream::overflow(RSASHA256Stream::traits::int_type c)
	{
		char *ibegin = out_buf_;
		char *iend = pptr();

		// mark the buffer as free
		setp(out_buf_, out_buf_ + BUFF_SIZE - 1);

		if (!std::char_traits<char>::eq_int_type(c, std::char_traits<char>::eof()))
		{
			//TODO writing one byte after the buffer?
			*iend++ = std::char_traits<char>::to_char_type(c);
		}

		// if there is nothing to send, just return
		if ((iend - ibegin) == 0)
		{
			return std::char_traits<char>::not_eof(c);
		}

		if (!_verify)
			// hashing
		{
			if (!EVP_SignUpdate(&_ctx, reinterpret_cast<unsigned char*>(out_buf_), iend - ibegin))
			{
				IBRCOMMON_LOGGER(critical) << "failed to feed data into the signature function" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
			}
		}
		else
		{
			if (!EVP_VerifyUpdate(&_ctx, reinterpret_cast<unsigned char*>(out_buf_), iend - ibegin))
			{
				IBRCOMMON_LOGGER(critical) << "failed to feed data into the verfication function" << IBRCOMMON_LOGGER_ENDL;
				ERR_print_errors_fp(stderr);
			}
		}

		return std::char_traits<char>::not_eof(c);
	}
}
