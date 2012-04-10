/*
 * SecurityKeyManager.h
 *
 *  Created on: 02.12.2010
 *      Author: morgenro
 */

#ifndef SECURITYKEYMANAGER_H_
#define SECURITYKEYMANAGER_H_

#include <ibrdtn/security/SecurityKey.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/DTNTime.h>
#include <ibrdtn/data/BundleString.h>
#include <ibrdtn/data/SDNV.h>
#include <ibrcommon/data/File.h>
#include <iostream>

#include <openssl/rsa.h>

namespace dtn
{
	namespace security
	{
		class SecurityKeyManager
		{
		public:
			class KeyNotFoundException : public ibrcommon::Exception
			{
			public:
				KeyNotFoundException(std::string what = "Requested key not found.") : ibrcommon::Exception(what)
				{};

				virtual ~KeyNotFoundException() throw() {};
			};

			static SecurityKeyManager& getInstance();

			virtual ~SecurityKeyManager();
			void initialize(const ibrcommon::File &path, const ibrcommon::File &ca, const ibrcommon::File &key);

			void prefetchKey(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type = dtn::security::SecurityKey::KEY_UNSPEC);

			bool hasKey(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type = dtn::security::SecurityKey::KEY_UNSPEC) const;
			dtn::security::SecurityKey get(const dtn::data::EID &ref, const dtn::security::SecurityKey::KeyType type = dtn::security::SecurityKey::KEY_UNSPEC) const throw (SecurityKeyManager::KeyNotFoundException);
			void store(const dtn::data::EID &ref, const std::string &data, const dtn::security::SecurityKey::KeyType type = dtn::security::SecurityKey::KEY_UNSPEC);

		private:
			SecurityKeyManager();

			/**
			Reads a private key into rsa
			@param filename the file where the key is stored
			@param rsa the rsa file
			@return zero if all is ok, something different from zero when something
			failed
			*/
			static int read_private_key(const ibrcommon::File &file, RSA ** rsa);

			/**
			Reads a public key into rsa
			@param filename the file where the key is stored
			@param rsa the rsa file
			@return zero if all is ok, something different from zero when something
			failed
			*/
			static int read_public_key(const ibrcommon::File &file, RSA ** rsa);


			static const std::string hash(const dtn::data::EID &eid);

			ibrcommon::File _path;
			ibrcommon::File _ca;
			ibrcommon::File _key;
		};
	}
}

#endif /* SECURITYKEYMANAGER_H_ */
