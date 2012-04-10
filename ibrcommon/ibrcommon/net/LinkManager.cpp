/*
 * LinkManager.cpp
 *
 *  Created on: 21.12.2010
 *      Author: morgenro
 */

#include "ibrcommon/config.h"
#include "ibrcommon/net/LinkManager.h"
#include "ibrcommon/thread/MutexLock.h"
#include <list>
#include <string>
#include <typeinfo>

#ifdef HAVE_LIBNL
#include "ibrcommon/net/NetLinkManager.h"
#else
#ifdef HAVE_LIBNL3
#include "ibrcommon/net/NetLink3Manager.h"
#else
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <stdlib.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <errno.h>
#include <string.h>
#endif
#endif

namespace ibrcommon
{
	LinkManager& LinkManager::getInstance()
	{
#ifdef HAVE_LIBNL
		static NetLinkManager lm;
#else
#ifdef HAVE_LIBNL3
		static NetLink3Manager lm;
#else
		static DefaultLinkManager lm;
#endif
#endif

		return lm;
	}

	void LinkManager::initialize()
	{
#ifdef HAVE_LIBNL
		try {
			dynamic_cast<NetLinkManager&>(getInstance()).start();
		} catch (const std::bad_cast&) { };
#else
#ifdef HAVE_LIBNL3
		try {
			dynamic_cast<NetLink3Manager&>(getInstance()).start();
		} catch (const std::bad_cast&) { };
#endif
#endif
	}

	void LinkManager::registerInterfaceEvent(const vinterface &iface, LinkManager::EventCallback *cb)
	{
		if (cb == NULL) return;
		ibrcommon::MutexLock l(_listener_mutex);

		std::set<LinkManager::EventCallback* > &ss = _listener[iface];
		ss.insert(cb);
	}

	void LinkManager::unregisterInterfaceEvent(const vinterface &iface, LinkManager::EventCallback *cb)
	{
		if (cb == NULL) return;
		ibrcommon::MutexLock l(_listener_mutex);

		std::set<LinkManager::EventCallback* > &ss = _listener[iface];

		ss.erase(cb);

		if (ss.empty())
		{
			_listener.erase(iface);
		}
	}

	void LinkManager::unregisterAllEvents(LinkManager::EventCallback *cb)
	{
		if (cb == NULL) return;

		try {
			ibrcommon::MutexLock l(_listener_mutex);

			for (std::map<vinterface, std::set<LinkManager::EventCallback* > >::iterator iter = _listener.begin(); iter != _listener.end(); iter++)
			{
				std::set<LinkManager::EventCallback* > &ss = iter->second;
				ss.erase(cb);
			}
		} catch (const ibrcommon::MutexException&) {
			// this happens if this method is called after destroying the object
			// and is normal at shutdown
		}
	}

	void LinkManager::raiseEvent(const LinkManagerEvent &lme)
	{
		// get the corresponding interface
		const vinterface &iface = lme.getInterface();

		// search for event subscriptions
		ibrcommon::MutexLock l(_listener_mutex);
		std::set<LinkManager::EventCallback* > &ss = _listener[iface];

		for (std::set<LinkManager::EventCallback* >::iterator iter = ss.begin(); iter != ss.end(); iter++)
		{
			try {
				(*iter)->eventNotify((LinkManagerEvent&)lme);
			} catch (const std::exception&) { };
		}
	}

	LinkManager::ExternalLinkManagerEvent::ExternalLinkManagerEvent(LinkManagerEvent::EventType type, const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr, unsigned int state)
	 : _type(type), _iface(iface), _addr(addr), _state(state)
	{ };

	LinkManager::ExternalLinkManagerEvent::~ExternalLinkManagerEvent()
	{ };

	const ibrcommon::vinterface& LinkManager::ExternalLinkManagerEvent::getInterface() const
	{
		return _iface;
	};

	const ibrcommon::vaddress& LinkManager::ExternalLinkManagerEvent::getAddress() const
	{
		return _addr;
	};

	unsigned int LinkManager::ExternalLinkManagerEvent::getState() const
	{
		return _state;
	};

	LinkManagerEvent::EventType LinkManager::ExternalLinkManagerEvent::getType() const
	{
		return _type;
	};

	void LinkManager::addressRemoved(const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr)
	{
		ExternalLinkManagerEvent evt(LinkManagerEvent::EVENT_ADDRESS_REMOVED, iface, addr);
		raiseEvent(evt);
	}

	void LinkManager::addressAdded(const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr)
	{
		ExternalLinkManagerEvent evt(LinkManagerEvent::EVENT_ADDRESS_ADDED, iface, addr);
		raiseEvent(evt);
	}

	DefaultLinkManager::DefaultLinkManager()
	{
	}

	DefaultLinkManager::~DefaultLinkManager()
	{
	}

	const std::string DefaultLinkManager::getInterface(int index) const
	{
#if defined HAVE_LIBNL || HAVE_LIBNL3
		throw ibrcommon::Exception("not implemented - use NetlinkManager instead");
#else
		struct ifaddrs *ifap = NULL;
		int status = getifaddrs(&ifap);
		int i = 0;
		std::string ret;

		if ((status != 0) || (ifap == NULL))
		{
				// error, return with default address
				throw ibrcommon::Exception("can not iterate through interfaces");
		}

		for (struct ifaddrs *iter = ifap; iter != NULL; iter = iter->ifa_next, i++)
		{
			if (index == i)
			{
				ret = iter->ifa_name;
				break;
			}
		}

		freeifaddrs(ifap);

		return ret;
#endif
	}

#if !( defined(HAVE_LIBNL) || defined(HAVE_LIBNL3) )
	char *get_ip_str(const struct sockaddr *sa, char *s, size_t maxlen)
	{
		switch(sa->sa_family) {
			case AF_INET:
				inet_ntop(AF_INET, &(((struct sockaddr_in *)sa)->sin_addr),
						s, maxlen);
				break;

			case AF_INET6:
				inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)sa)->sin6_addr),
						s, maxlen);
				break;

			default:
				strncpy(s, "Unknown AF", maxlen);
				return NULL;
		}

		return s;
	}

	int get_scope_id(struct sockaddr *sa)
	{
		switch(sa->sa_family)
		{
			case AF_INET6:
			{
				struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)sa;
				return sin6->sin6_scope_id;
			}

			case AF_INET:
			{
				return 0;
			}

			default:
			{
				return 0;
			}
		}
	}
#endif

	const std::list<vaddress> DefaultLinkManager::getAddressList(const vinterface &iface, const vaddress::Family f)
	{
		std::list<vaddress> ret;

#if !( defined(HAVE_LIBNL) || defined(HAVE_LIBNL3) )
		struct ifaddrs *ifap = NULL;
		int status = getifaddrs(&ifap);
		int i = 0;

		if ((status != 0) || (ifap == NULL))
		{
				// error, return with default address
				throw ibrcommon::Exception("can not iterate through interfaces");
		}

		for (struct ifaddrs *iter = ifap; iter != NULL; iter = iter->ifa_next, i++)
		{
				if (iter->ifa_addr == NULL) continue;
				if ((iter->ifa_flags & IFF_UP) == 0) continue;

				// cast to a sockaddr
				sockaddr *addr = iter->ifa_addr;

				// compare the address family
				if ((f != vaddress::VADDRESS_UNSPEC) && (addr->sa_family != f)) continue;

				// check the name of the interface
				if (string(iter->ifa_name).compare(iface.toString()) != 0) continue;

				char buf[INET6_ADDRSTRLEN+5];
				char *name = get_ip_str(iter->ifa_addr, buf, INET6_ADDRSTRLEN+5);

				if (name != NULL)
				{
					std::string name_str(name);
					if (get_scope_id(addr) == 0)
					{
						ret.push_back(
								vaddress(vaddress::Family(iter->ifa_addr->sa_family), name_str)
								);
					}
					else
					{
						ret.push_back(
								vaddress(vaddress::Family(iter->ifa_addr->sa_family), name_str, i)
								);
					}
				}


		}
		freeifaddrs(ifap);
#endif

		return ret;
	}
}
