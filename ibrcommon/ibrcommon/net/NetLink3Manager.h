/*
 * NetLink3Manager.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef NETLINK3MANAGER_H_
#define NETLINK3MANAGER_H_

#include "ibrcommon/net/LinkManager.h"
#include "ibrcommon/thread/Mutex.h"
#include "ibrcommon/thread/Thread.h"
#include "ibrcommon/net/vsocket.h"

#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/route/link.h>
#include <netlink/msg.h>

namespace ibrcommon
{
	class NetLink3ManagerEvent : public LinkManagerEvent
	{
	public:
		NetLink3ManagerEvent(struct nl_msg *msg);
		virtual ~NetLink3ManagerEvent();

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

	class NetLink3Manager : public ibrcommon::LinkManager, public ibrcommon::JoinableThread
	{
		friend class LinkManager;

	public:
		virtual ~NetLink3Manager();

		const std::string getInterface(int index) const;
		const std::list<vaddress> getAddressList(const vinterface &iface, const vaddress::Family f);

		class parse_exception : public Exception
		{
		public:
			parse_exception(string error) : Exception(error)
			{};
		};

		void callback(const NetLink3ManagerEvent &evt);

	protected:
		void run();
		void __cancellation();

	private:
		NetLink3Manager();

		struct nl_sock *_nl_notify_sock;
		struct nl_sock *_nl_query_sock;

		// local link cache
		struct nl_cache *_link_cache;
		struct nl_cache *_addr_cache;

		// mutex for the link cache
		ibrcommon::Mutex _call_mutex;

		bool _refresh_cache;
		bool _running;

		ibrcommon::vsocket *_sock;
	};
} /* namespace ibrcommon */
#endif /* NETLINK3MANAGER_H_ */
