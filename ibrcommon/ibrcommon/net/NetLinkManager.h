/*
 * NetLinkManager.h
 *
 *  Created on: 21.12.2010
 *      Author: morgenro
 */

#ifndef NETLINKMANAGER_H_
#define NETLINKMANAGER_H_

#include "ibrcommon/net/LinkManager.h"
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/thread/Mutex.h"
#include "ibrcommon/thread/Conditional.h"
#include "ibrcommon/thread/Thread.h"

//#include <netinet/in.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>

#include <map>

namespace ibrcommon
{
	class NetLinkManagerEvent : public LinkManagerEvent
	{
	public:
		NetLinkManagerEvent(int fd);
		virtual ~NetLinkManagerEvent();

		virtual const ibrcommon::vinterface& getInterface() const;
		virtual const ibrcommon::vaddress& getAddress() const;
		virtual unsigned int getState() const;
		virtual EventType getType() const;

		virtual bool isWirelessExtension() const;

		void debug() const;
		const std::string toString() const;

	private:
		EventType _type;
		unsigned int _state;
		bool _wireless;
		ibrcommon::vinterface _interface;
		ibrcommon::vaddress _address;
		std::map<int, std::string> _attributes;
	};

	class NetLinkManager : public ibrcommon::LinkManager, public ibrcommon::JoinableThread
	{
		friend class LinkManager;

	public:
		virtual ~NetLinkManager();

		const std::string getInterface(int index) const;
		const std::list<vaddress> getAddressList(const vinterface &iface, const vaddress::Family f);

		class parse_exception : public Exception
		{
		public:
			parse_exception(string error) : Exception(error)
			{};
		};

	protected:
		void run();
		void __cancellation();

	private:
		NetLinkManager();

		std::map<int, std::string> read_nlmsg(int fd);

		ibrcommon::Mutex _call_mutex;

		struct nl_handle *_handle;
		struct nl_cache *_link_cache;
		struct nl_cache *_addr_cache;

		bool _initialized;
		ibrcommon::vsocket *_sock;

		bool _refresh_cache;
	};
}

#endif /* NETLINKMANAGER_H_ */
