/*
 * TLSExceptions.h
 *
 *  Created on: Mar 24, 2011
 *      Author: roettger
 */

#ifndef TLSEXCEPTIONS_H_
#define TLSEXCEPTIONS_H_

#include "ibrcommon/Exceptions.h"

namespace ibrcommon
{
	class TLSException : public ibrcommon::Exception
	{
	public:
		TLSException(const std::string &what) throw() : Exception(what)
		{
		}
	};

	class ContextCreationException : public TLSException
	{
	public:
		ContextCreationException(const std::string &what) throw() : TLSException(what)
		{
		};
	};

	class SSLCreationException : public TLSException
	{
	public:
		SSLCreationException(const std::string &what) throw() : TLSException(what)
		{
		};
	};

	class BIOCreationException : public TLSException
	{
	public:
		BIOCreationException(const std::string &what) throw() : TLSException(what)
		{
		};
	};

	class TLSHandshakeException : public TLSException
	{
	public:
		TLSHandshakeException(const std::string &what) throw() : TLSException(what)
		{
		};
	};

	class TLSNotInitializedException : public TLSException
	{
	public:
		TLSNotInitializedException(const std::string &what) throw() : TLSException(what)
		{
		};
	};

	class TLSCertificateFileException : public TLSException
	{
	public:
		TLSCertificateFileException(const std::string &what) throw() : TLSException(what)
		{
		};
	};

	class TLSKeyFileException : public TLSException
	{
	public:
		TLSKeyFileException(const std::string &what) throw() : TLSException(what)
		{
		};
	};

	class TLSCertificateVerificationException : public TLSException
	{
	public:
		TLSCertificateVerificationException(const std::string &what) throw() : TLSException(what)
		{
		};
	};
}

#endif /* TLSEXCEPTIONS_H_ */
