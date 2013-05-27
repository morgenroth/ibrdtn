/*
 * TLSStream.h
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Stephen Roettger <roettger@ibr.cs.tu-bs.de>
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

#ifndef TLSSTREAM_H_
#define TLSSTREAM_H_

#include <streambuf>
#include <iostream>
#include <memory>
#include <vector>
#include <openssl/ssl.h>
#include "ibrcommon/thread/Mutex.h"
#include "ibrcommon/data/File.h"
#include "ibrcommon/ssl/iostreamBIO.h"

namespace ibrcommon
{
	/*!
	 * \brief A Stream Class that adds Signatures and Encryption through TLS.
	 *
	 * The Stream passes data from and to the underlying Stream unchanged(unencrypted) until activate() is called.
	 * init() has to be called before the first usage.
	 */
	class TLSStream : public std::basic_streambuf<char, std::char_traits<char> >, public std::iostream
	{
		static const std::string TAG;

	public:
		typedef std::char_traits<char> traits;

		/*!
		 * \brief The TLSStream Constructor
		 * \param stream the underlying Stream to read from / write to
		 * \param server states, if TLS is used in client or server mode (true for server, false for client)
		 */
		TLSStream(std::iostream *stream);
		/*!
		 * \brief The default Destructor
		 */
		virtual ~TLSStream();

		/**
		 * If true, set the connection mode to server.
		 * @param val
		 */
		void setServer(bool val);

		/*!
		 * \brief Initializes the TLSStream class
		 * \param certificate The certificate for the private Key
		 * \param privateKey The private Key to use with openSSL
		 * \param trustedCAPath A directory containing certificates that are trusted. These are also used to build the own certificate chain.
		 * \param enableEncryption True if encryption shall be enabled. Otherwise only authentication is enabled.
		 *
		 * In particular, this function initializes the used openSSL Context.
		 * The certificate directory has to hold certificates files with hashed names created by c_rehash (from the openssl library).
		 * \warning Beware that the certificate path does not have certificates valid and invalid certificates mixed with the same subject, openssl will only use the first that is found.
		 * \warning on default, encryption is disabled and the stream does only provide authentication
		 */
	    static void init(X509 *certificate, EVP_PKEY *privateKey, ibrcommon::File trustedCAPath, bool enableEncryption = false);

	    /*!
	     * \brief Removes the SSL_CTX to allow a new init()
	     */
	    static void flushInitialization();

		/*!
		 * \brief checks if the Class is already initialized.
		 * \return true if its initialized, false otherwise
		 */
	    static bool isInitialized();

	    /*!
	     * \brief Closes the TLS Connection.
	     * \warning The underlying Stream is not closed by this function.
	     */
	    void close();

		/// The size of the input and output buffers.
		static const size_t BUFF_SIZE = 5120;

		/*!
		 * \return the X509 certificate of the peer
		 * \warning the caller has to check the identity in the certificate
		 */
		X509 *activate();

	protected:
		virtual int sync();
		virtual traits::int_type overflow(traits::int_type = traits::eof());
		virtual traits::int_type underflow();

	private:
		std::string log_error_msg(int errnumber);

		static bool _initialized;
		/* this second initialized variable is needed, because init() can fail and SSL_library_init() is not reentrant. */
		static bool _SSL_initialized;
		static ibrcommon::Mutex _initialization_lock;

		bool _activated;
		ibrcommon::Mutex _activation_lock;

		// Input buffer
		std::vector<char> in_buf_;
		// Output buffer
		std::vector<char> out_buf_;

		std::iostream *_stream;
		/* indicates if this node is the server in the underlying tcp connection */
		bool _server;

		static SSL_CTX *_ssl_ctx;
		SSL *_ssl;
		X509 *_peer_cert;
		iostreamBIO *_iostreamBIO;
	};
}

#endif /* TLSSTREAM_H_ */
