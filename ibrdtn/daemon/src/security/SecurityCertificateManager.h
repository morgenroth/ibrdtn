/*
 * SecurityCertificateManager.h
 *
 *  Created on: Apr 2, 2011
 *      Author: roettger
 */

#ifndef SECURITYCERTIFICATEMANAGER_H_
#define SECURITYCERTIFICATEMANAGER_H_

#include "core/Event.h"
#include "Component.h"

#include <ibrcommon/data/File.h>
#include <ibrcommon/thread/Mutex.h>

#include <ibrdtn/data/EID.h>

#include <openssl/ssl.h>
#include <string>

namespace dtn {

namespace security {

/*!
 * \brief This class represents an event that is thrown if the Initialization of the SecurityCertificateManager succeeded.
 */
class CertificateManagerInitEvent : public dtn::core::Event{
public:
	virtual ~CertificateManagerInitEvent();

	/* from Event */
	virtual const std::string getName() const;
	virtual std::string toString() const;

	static const std::string className;

	/*!
	 * \brief this function raises a new CertificateManagerInitEvent
	 * \param certificate the X509 certificate, the manager was initialized with
	 * \param privateKey the private key
	 * \param trustedCAPath directory with trusted certificates
	 */
	static void raise(X509 * certificate, EVP_PKEY * privateKey, const ibrcommon::File &trustedCAPath);

	X509 * const certificate;
	EVP_PKEY * const privateKey;
	const ibrcommon::File trustedCAPath;

private:
	CertificateManagerInitEvent(X509 * certificate, EVP_PKEY * privateKey, const ibrcommon::File &trustedCAPath);
};

/*!
 * \brief This class is a manager to handle certificates
 */
class SecurityCertificateManager : public dtn::daemon::Component {
public:
	SecurityCertificateManager();
	virtual ~SecurityCertificateManager();

	//void addChainCertificate(ibrcommon::File &certificate);
	/*!
	 * \brief Validates if the CommonName in the given X509 certificate corresponds to the given EID
	 * \param certificate The Certificate.
	 * \param eid The EID of the sender.
	 * \return returns true if the EID fits, false otherwise
	 */
	static bool validateSubject(X509 *certificate, const dtn::data::EID &eid);

	/*!
	 * \brief checks if this class has already been initialized with a certificate and private key
	 * \return true if it is initialized, false otherwise
	 */
	bool isInitialized();

	/*!
	 * \brief retrieve the saved certificate
	 * \return The certificate.
	 * \warning Check isInitialized() first, before calling this function
	 */
    X509 *getCert();
	/*!
	 * \brief retrieve the saved private key
	 * \return The private key as an EVP_PKEY pointer (OpenSSL).
	 * \warning Check isInitialized() first, before calling this function
	 */
    EVP_PKEY *getPrivateKey();
	/*!
	 * \brief retrieve the saved directory holding trusted certificates
	 * \return The directory
	 */
    ibrcommon::File getTrustedCAPath() const;

    /* functions from Component */
    virtual void initialize();
	virtual void startup();
	virtual void terminate();
	virtual const std::string getName() const;

private:
	ibrcommon::Mutex _initialization_lock;
	bool _initialized;

	X509 *_cert;
	EVP_PKEY *_privateKey;
	ibrcommon::File _trustedCAPath;
};

}

}

#endif /* SECURITYCERTIFICATEMANAGER_H_ */
