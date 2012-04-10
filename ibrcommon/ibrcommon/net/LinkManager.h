/*
 * LinkManager.h
 *
 *  Created on: 21.12.2010
 *      Author: morgenro
 */

#ifndef LINKMANAGER_H_
#define LINKMANAGER_H_

#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vaddress.h"
#include "ibrcommon/thread/Mutex.h"
#include <string>
#include <map>
#include <set>

namespace ibrcommon
{
	class LinkManagerEvent
	{
	public:
		enum EventType
		{
			EVENT_UNKOWN,
			EVENT_LINK_STATE,
			EVENT_ADDRESS_REMOVED,
			EVENT_ADDRESS_ADDED
		};

		virtual ~LinkManagerEvent() {};

		virtual const ibrcommon::vinterface& getInterface() const = 0;
		virtual const ibrcommon::vaddress& getAddress() const = 0;
		virtual unsigned int getState() const = 0;
		virtual EventType getType() const = 0;
	};

	class LinkManager
	{
	public:
		class EventCallback
		{
		public:
			virtual ~EventCallback() {};
			virtual void eventNotify(const LinkManagerEvent&) = 0;
		};

		class ExternalLinkManagerEvent : public LinkManagerEvent
		{
		public:
			ExternalLinkManagerEvent(LinkManagerEvent::EventType type, const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr, unsigned int state = 0);
			virtual ~ExternalLinkManagerEvent();

			virtual const ibrcommon::vinterface& getInterface() const;
			virtual const ibrcommon::vaddress& getAddress() const;
			virtual unsigned int getState() const;
			virtual LinkManagerEvent::EventType getType() const;

		private:
			LinkManagerEvent::EventType _type;
			const ibrcommon::vinterface _iface;
			const ibrcommon::vaddress _addr;
			unsigned int _state;
		};

		virtual ~LinkManager() {};

		virtual const std::string getInterface(int index) const = 0;
		virtual const std::list<vaddress> getAddressList(const vinterface &iface, const vaddress::Family f) = 0;

		virtual void registerInterfaceEvent(const vinterface&, LinkManager::EventCallback*);
		virtual void unregisterInterfaceEvent(const vinterface&, LinkManager::EventCallback*);
		virtual void unregisterAllEvents(LinkManager::EventCallback*);

		void addressAdded(const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr);
		void addressRemoved(const ibrcommon::vinterface &iface, const ibrcommon::vaddress &addr);

		static LinkManager& getInstance();
		static void initialize();

	protected:
		void raiseEvent(const LinkManagerEvent &lme);

		ibrcommon::Mutex _listener_mutex;
		std::map<ibrcommon::vinterface, std::set<LinkManager::EventCallback* > > _listener;
	};

	class DefaultLinkManager : public LinkManager
	{
	public:
		DefaultLinkManager();
		virtual ~DefaultLinkManager();

		const std::string getInterface(int index) const;
		const std::list<vaddress> getAddressList(const vinterface &iface, const vaddress::Family f);
	};
}

#endif /* LINKMANAGER_H_ */
