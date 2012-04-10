/*
 * vaddress.h
 *
 *  Created on: 14.12.2010
 *      Author: morgenro
 */

#ifndef VADDRESS_H_
#define VADDRESS_H_

#include "ibrcommon/Exceptions.h"
#include <sys/socket.h>
#include <netdb.h>

namespace ibrcommon
{
	class vaddress
	{
		public:
			class address_not_set : public Exception
			{
			public:
				address_not_set(string error = "address is not specified") : Exception(error)
				{};
			};

			class family_not_set : public Exception
			{
			public:
				family_not_set(string error = "family is not specified") : Exception(error)
				{};
			};

			enum Family
			{
				VADDRESS_UNSPEC = AF_UNSPEC,
				VADDRESS_INET = AF_INET,
				VADDRESS_INET6 = AF_INET6,
				VADDRESS_UNIX = AF_UNIX
			};

			vaddress(const std::string &address);
			vaddress(const Family &family = VADDRESS_INET);
			vaddress(const Family &family, const std::string &address, const bool broadcast = false);
			vaddress(const Family &family, const std::string &address, const int iface, const bool broadcast = false);
			virtual ~vaddress();

			Family getFamily() const;
			const std::string get(bool internal = true) const;
			bool isBroadcast() const;

			/**
			 * Checks whether a given address is a multicast address or not
			 * @param address The address to check.
			 * @return True, if the address is a multicast address.
			 */
			bool isMulticast() const;

			bool operator!=(const vaddress &obj) const;
			bool operator==(const vaddress &obj) const;

			const std::string toString() const;

			struct addrinfo* addrinfo(struct addrinfo *hints) const;
			struct addrinfo* addrinfo(struct addrinfo *hints, unsigned int port) const;

			static const std::string strip_netmask(const std::string &data);

		private:
			static const std::string __REGEX_IPV6_ADDRESS__;
			static const std::string __REGEX_IPV4_ADDRESS__;

			Family _family;
			std::string _address;
			bool _broadcast;
			int _iface;
	};
}

#endif /* VADDRESS_H_ */
