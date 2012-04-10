/*
 * TLSStream.cpp
 *
 *  Created on: Mar 24, 2011
 *      Author: roettger
 */

#include "TLSStream.h"

#include "ibrcommon/thread/Mutex.h"
#include "ibrcommon/thread/MutexLock.h"

#include "ibrcommon/Logger.h"

#include "ibrcommon/net/iostreamBIO.h"
#include "ibrcommon/TLSExceptions.h"

#include "ibrcommon/data/File.h"

#include <openssl/ssl.h>
#include <openssl/err.h>

namespace ibrcommon
{
	static const int ERR_BUF_SIZE = 256;

	SSL_CTX *TLSStream::_ssl_ctx = NULL;
	bool TLSStream::_initialized = false;
	bool TLSStream::_SSL_initialized = false;
	ibrcommon::Mutex TLSStream::_initialization_lock;

	TLSStream::TLSStream(std::iostream *stream) :
		_stream(stream), in_buf_(new char[BUFF_SIZE]), out_buf_(new char[BUFF_SIZE]),
		_server(false), _activated(false), iostream(this), _ssl(NULL), _iostreamBIO(NULL)
	{
		/* basic_streambuf related initialization */
		// Initialize get pointer.  This should be zero so that underflow is called upon first read.
		setg(0, 0, 0);
		setp(out_buf_, out_buf_ + BUFF_SIZE - 1);
	}

	TLSStream::~TLSStream()
	{
		if(_ssl) SSL_free(_ssl);
		delete[] in_buf_;
		delete[] out_buf_;
		if (_iostreamBIO != NULL) delete _iostreamBIO;
	}

	void TLSStream::setServer(bool val)
	{
		_server = val;
	}

	X509 *TLSStream::activate()
	{
		int error;

		ibrcommon::MutexLock l(_activation_lock);
		if(_activated){
			IBRCOMMON_LOGGER(info) << "TLS has already been activated." << IBRCOMMON_LOGGER_ENDL;
			return _peer_cert;
		}

		/* check if TLSStream has already been initialized */
		if(!_initialized){
			IBRCOMMON_LOGGER(critical) << "TLSStream: TLSStream has not been initialized. Aborting activation." << IBRCOMMON_LOGGER_ENDL;
			throw TLSNotInitializedException("TLSStream is not initialized.");
		}

		/* create ssl object */
		_ssl = SSL_new(_ssl_ctx);
		if(!_ssl){
			char err_buf[ERR_BUF_SIZE];
			ERR_error_string_n(ERR_get_error(), err_buf, ERR_BUF_SIZE);
			err_buf[ERR_BUF_SIZE - 1] = '\0';
			IBRCOMMON_LOGGER(critical) << "TLSStream: SSL Object creation failed: " << err_buf << IBRCOMMON_LOGGER_ENDL;
			throw SSLCreationException(err_buf);
		}

		/* set the ssl type (client or server) */
		if(_server){
			SSL_set_accept_state(_ssl);
		} else {
			SSL_set_connect_state(_ssl);
		}

		/* create and assign BIO object */
		try{
			_iostreamBIO = new iostreamBIO(_stream);
			//_bio = iostreamBIO::getBIO(_stream);
		} catch(iostreamBIO::BIOException &ex){
			throw BIOCreationException(ex.what());
		}
		try{
			SSL_set_bio(_ssl, _iostreamBIO->getBIO(), _iostreamBIO->getBIO());
		} catch(BIOCreationException &ex){
			if (_iostreamBIO != NULL) delete _iostreamBIO;
			_iostreamBIO = NULL;
			SSL_free(_ssl);
			_ssl = NULL;
			throw;
		}

		/* perform TLS Handshake */
		error = SSL_do_handshake(_ssl);
		if(error <= 0){ /* on 0 the handshake failed but was shut down controlled */
			int errcode = SSL_get_error(_ssl, error);
			log_error("TLS handshake failed", errcode);

			/* cleanup */
			if (_iostreamBIO != NULL) delete _iostreamBIO;
			_iostreamBIO = NULL;
			SSL_free(_ssl);
			_ssl = NULL;
			throw TLSHandshakeException(log_error_msg(errcode));
		}

		_peer_cert = SSL_get_peer_certificate(_ssl);
		if((error = SSL_get_verify_result(_ssl)) != X509_V_OK || _peer_cert == NULL){
			IBRCOMMON_LOGGER(error) << "TLSStream: Peer certificate could not be verified. Error: " << error << ". Error codes are documented in \"man 1 verify\"." << IBRCOMMON_LOGGER_ENDL;
			if(_peer_cert){
				X509_free(_peer_cert);
				_peer_cert = NULL;
			}
			if (_iostreamBIO != NULL) delete _iostreamBIO;
			_iostreamBIO = NULL;
			SSL_free(_ssl);
			_ssl = NULL;
			std::stringstream ss; ss << "TLSStream: Certificate verification error " << error << ".";
			throw TLSCertificateVerificationException(ss.str());
		}

		_activated = true;

		return _peer_cert;
	}

	TLSStream::traits::int_type TLSStream::underflow()
	{
		streamsize num_bytes;
		/* get data from tcpstream */

		if(_activated)
		{
			/* decrypt */
			num_bytes = SSL_read(_ssl, in_buf_, BUFF_SIZE);
			if(num_bytes == 0){
				/* connection closed */
				if(SSL_get_error(_ssl, num_bytes) == SSL_ERROR_ZERO_RETURN){
					/* clean shutdown */
					close();
				} else {
					log_debug("underflow()", SSL_get_error(_ssl, num_bytes));
				}
				return traits::eof();
			} else if(num_bytes < 0){
				log_debug("underflow()", SSL_get_error(_ssl, num_bytes));
				return traits::eof();
			}
		}
		else
		{
			try{
				/* make sure to read at least 1 byte and then read as much as we can */
				num_bytes = _stream->read(in_buf_, 1).readsome(in_buf_+1, BUFF_SIZE-1) + 1;
			} catch(ios_base::failure &ex){
				/* ignore, the specific bits are checked later */
			}
			/* error checking, these can probably not happen because the ConnectionClosedException is thrown first */
			if(_stream->eof()){
				return traits::eof();
			}
			if(_stream->fail()){
				IBRCOMMON_LOGGER_DEBUG(15) << "TLSStream: underlying stream failed while reading." << IBRCOMMON_LOGGER_ENDL;
				return traits::eof();
			}
			if(_stream->bad()){
				IBRCOMMON_LOGGER_DEBUG(15) << "TLSStream: underlying stream went bad while reading." << IBRCOMMON_LOGGER_ENDL;
				return traits::eof();
			}
		}

		setg(in_buf_, in_buf_, in_buf_ + num_bytes);

		return traits::not_eof((unsigned char)in_buf_[0]);
	}

	TLSStream::traits::int_type TLSStream::overflow(traits::int_type c)
	{
		streamsize num_bytes;
		char *ibegin = out_buf_;
		char *iend = pptr();

		// mark the buffer as free
		setp(out_buf_, out_buf_ + BUFF_SIZE - 1);

		// append the last character
		if(!traits::eq_int_type(c, traits::eof())) {
			*iend++ = traits::to_char_type(c);
		}

		// if there is nothing to send, just return
		if ((iend - ibegin) == 0)
		{
			return traits::not_eof(c);
		}

		if(_activated)
		{
			/* use ssl to send */
			num_bytes = SSL_write(_ssl, out_buf_, (iend - ibegin));
			if(num_bytes == 0){
				/* connection closed */
				return traits::eof();
			} else if(num_bytes < 0){
				log_debug("overflow()", SSL_get_error(_ssl, num_bytes));
				return traits::eof();
			}
		}
		else
		{
			try{
				_stream->write(out_buf_, (iend - ibegin));
			} catch(ios_base::failure &ex){
				/* ignore, the badbit is checked instead */
			}
			if(_stream->bad()){
				IBRCOMMON_LOGGER_DEBUG(15) << "TLSStream: underlying stream went bad while writing." << IBRCOMMON_LOGGER_ENDL;
				return traits::eof();
			}
		}

		return traits::not_eof(c);
	}

	void TLSStream::init(X509 *certificate, EVP_PKEY *privateKey, ibrcommon::File trustedCAPath, bool enableEncryption)
	{
		ibrcommon::MutexLock l(_initialization_lock);
		if(_initialized){
			IBRCOMMON_LOGGER(info) << "TLSStream: TLS has already been initialized." << IBRCOMMON_LOGGER_ENDL;
			return;
		}

		/* openssl initialization */
		/* the if block is needed because SSL_library_init() is not reentrant */
		if(!_SSL_initialized){
			SSL_load_error_strings();
			SSL_library_init();
			ERR_load_BIO_strings();
			ERR_load_SSL_strings();
			_SSL_initialized = true;
		}


		/* create ssl context and throw exception if it fails */
		_ssl_ctx = SSL_CTX_new(TLSv1_method());
		if(!_ssl_ctx){
			char err_buf[ERR_BUF_SIZE];
			ERR_error_string_n(ERR_get_error(), err_buf, ERR_BUF_SIZE);
			err_buf[ERR_BUF_SIZE - 1] = '\0';
			IBRCOMMON_LOGGER(critical) << "TLSStream: TLS/SSL Context Object creation failed: " << err_buf << IBRCOMMON_LOGGER_ENDL;
			throw ContextCreationException(err_buf);
		}

		/* set verification mode */
		/* client and server require a valid certificate or the handshake fails */
		SSL_CTX_set_verify(_ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);

		/* load the trusted certificates from CAPath */
		if(trustedCAPath.isDirectory()){
			if(SSL_CTX_load_verify_locations(_ssl_ctx, NULL, trustedCAPath.getPath().c_str()) != 1){
			   IBRCOMMON_LOGGER(error) << "TLSStream: SSL_CTX_load_verify_locations failed." << IBRCOMMON_LOGGER_ENDL;
			}
		} else {
			IBRCOMMON_LOGGER(info) << "TLSStream: TLS initialization without trusted certificates." << IBRCOMMON_LOGGER_ENDL;
		}

		/* load the certificate and private key
		 * the certificate chain will be completed by openssl with certificates from CAPath */
		if(!certificate || SSL_CTX_use_certificate(_ssl_ctx, certificate) != 1){
			IBRCOMMON_LOGGER(critical) << "TLSStream: Could not load the certificate." << IBRCOMMON_LOGGER_ENDL;
			SSL_CTX_free(_ssl_ctx);
			_ssl_ctx = NULL;
			throw TLSCertificateFileException("Could not load the certificate.");
		}
		if(!privateKey || SSL_CTX_use_PrivateKey(_ssl_ctx, privateKey) != 1){
			IBRCOMMON_LOGGER(critical) << "TLSStream: Could not load the private key." << IBRCOMMON_LOGGER_ENDL;
			SSL_CTX_free(_ssl_ctx);
			_ssl_ctx = NULL;
			throw TLSKeyFileException("Could not load the private key.");
		}

		if(trustedCAPath.isDirectory()){
			std::list<ibrcommon::File> files;
			if(trustedCAPath.getFiles(files) == 0){
				/* success */
				for(std::list<ibrcommon::File>::iterator it = files.begin();
						it != files.end(); ++it){
					if(it->getType() == DT_LNK || it->isSystem() || it->isDirectory())
						continue;

					X509 *cert = NULL;
					FILE *fp = fopen(it->getPath().c_str(), "r");
					if(fp && PEM_read_X509(fp, &cert, NULL, NULL)){
						if(SSL_CTX_add_client_CA(_ssl_ctx, cert) != 1){
							IBRCOMMON_LOGGER(error) << "TLSStream: could not add client CA from file: " << it->getPath() << "." << IBRCOMMON_LOGGER_ENDL;
						}
					} else {
						IBRCOMMON_LOGGER(error) << "TLSStream: could not read certificate from " << it->getPath() << "." << IBRCOMMON_LOGGER_ENDL;
					}
				}
			} else {
				IBRCOMMON_LOGGER(error) << "TLSStream: Could not get files from CADirectory." << IBRCOMMON_LOGGER_ENDL;
			}
		}

		if(!enableEncryption){
			if(!SSL_CTX_set_cipher_list(_ssl_ctx, "eNULL")){
				IBRCOMMON_LOGGER(critical) << "TLSStream: Could not set the cipherlist." << IBRCOMMON_LOGGER_ENDL;
			}
		}

		_initialized = true;
		IBRCOMMON_LOGGER(info) << "TLSStream: Initialization succeeded." << IBRCOMMON_LOGGER_ENDL;
	}

	void TLSStream::flushInitialization()
	{
		ibrcommon::MutexLock l(_initialization_lock);
		if(!_initialized)
			return;

		/* remove the SSL Context */
		if(_ssl_ctx){
			SSL_CTX_free(_ssl_ctx);
			_ssl_ctx = NULL;
		}

		_initialized = false;
	}

	bool TLSStream::isInitialized(){
		return _initialized;
	}

	void TLSStream::close(){
		if(_ssl && _stream->good()){
			int ret;
			if((ret = SSL_shutdown(_ssl)) == -1){
				log_debug("SSL_shutdown error", SSL_get_error(_ssl, ret));
			} else if(ret == 0){
				/* try again */
				if((ret = SSL_shutdown(_ssl)) == -1){
					/* this can happen for example if the peer does not send a shutdown message */
					log_debug("SSL_shutdown error (2)", SSL_get_error(_ssl, ret));
				}
			}
		}
		IBRCOMMON_LOGGER_DEBUG(25) << "TLS Connection closed." << IBRCOMMON_LOGGER_ENDL;
	}

	int TLSStream::sync()
	{
		int ret = traits::eq_int_type(this->overflow(
				traits::eof()), traits::eof()) ? -1
				: 0;

		/* flush underlying stream */
		_stream->flush();

		return ret;
	}

	void TLSStream::log_error(std::string tag, int errnumber)
	{
		IBRCOMMON_LOGGER(error) << "[TLSStream] " << tag << " " << log_error_msg(errnumber) << IBRCOMMON_LOGGER_ENDL;
	}

	void TLSStream::log_debug(std::string tag, int errnumber)
	{
		IBRCOMMON_LOGGER_DEBUG(25) << "[TLSStream] " << tag << " " << log_error_msg(errnumber) << IBRCOMMON_LOGGER_ENDL;
	}

	std::string TLSStream::log_error_msg(int errnumber)
	{
		std::string ret;

		switch(errnumber){
		case SSL_ERROR_NONE:
			ret = "No Error.";
			break;
		case SSL_ERROR_ZERO_RETURN:
			ret = "TLS Connection has been closed.";
			break;
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE:
			ret = "WANT_READ/WANT_WRITE";
			break;
		case SSL_ERROR_WANT_CONNECT:
		case SSL_ERROR_WANT_ACCEPT:
			ret = "WANT_CONNECT/WANT_ACCEPT";
			break;
		case SSL_ERROR_WANT_X509_LOOKUP:
			ret = "WANT_X509_LOOKUP";
			break;
		case SSL_ERROR_SYSCALL:
			ret = "Syscall error.";
			break;
		case SSL_ERROR_SSL:{
			char err_buf[ERR_BUF_SIZE];
			ERR_error_string_n(ERR_get_error(), err_buf, ERR_BUF_SIZE);
			err_buf[ERR_BUF_SIZE - 1] = '\0';
			std::stringstream ss; ss << "SSL error: \"" << err_buf << "\".";
			ret = ss.str();
			break;
		}
		default:{
			std::stringstream ss; ss << "Unknown error code " << errnumber << ".";
			ret = ss.str();
			break;
		}
		}

		return ret;
	}

}
