/*
 * LinkRequester.cpp
 *
 *  Created on: Sep 9, 2013
 *      Author: goltzsch
 */

#include "ibrcommon/link/LinkMonitor.h"
#include "ibrcommon/link/LinkEvent.h"
#include "ibrcommon/thread/Conditional.h"
#include "ibrcommon/Logger.h"
#include <stdio.h>


#include <algorithm>

namespace ibrcommon {
	LinkMonitor::LinkMonitor(LinkManager *lm, size_t link_request_interval) : _lm(lm), _running(true)
	{
	}

	LinkMonitor::~LinkMonitor()
	{
		join();
	}

	void LinkMonitor::run() throw()
	{
		ibrcommon::Conditional cond;
		ibrcommon::MutexLock l(cond);
		cond.signal(true);
		while(_running)
		{

			IBRCOMMON_LOGGER_DEBUG_TAG("LinkMonitor",5) << "LOOP interval:" << _link_request_interval << IBRCOMMON_LOGGER_ENDL;
			std::set<vinterface> ifaces = _lm->getMonitoredInterfaces();
			for(std::set<vinterface>::iterator iface_iter = ifaces.begin(); iface_iter != ifaces.end(); iface_iter++)
			{
				std::list<vaddress> new_addresses = _lm->getAddressList(*iface_iter);
				std::set<vaddress> old_addresses;
				if(_iface_adr_map.size() > 0)
					old_addresses = _iface_adr_map.at(*iface_iter);



				new_addresses.sort();
				std::list<vaddress> addresses;

				LinkEvent::Action action = LinkEvent::ACTION_UNKOWN;

				//find deleted addresses
				std::set_difference(old_addresses.begin(),old_addresses.end(),new_addresses.begin(),new_addresses.end(), std::back_inserter(addresses));
				for(std::list<vaddress>::iterator adr_iter = addresses.begin(); adr_iter != addresses.end(); adr_iter++)
				{
						_iface_adr_map[*(iface_iter)].erase(*adr_iter);
						action = LinkEvent::ACTION_ADDRESS_REMOVED;
						IBRCOMMON_LOGGER_DEBUG_TAG("LinkMonitor", 5) << "address REMOVED:" << (*adr_iter).address() << " on interface " << (*iface_iter).toString() <<  IBRCOMMON_LOGGER_ENDL;
						LinkEvent lme(action, *(iface_iter), *(adr_iter));;
						_lm->raiseEvent(lme);
				}

				addresses.clear();

				//find added addresses
				std::set_difference(new_addresses.begin(),new_addresses.end(),old_addresses.begin(),old_addresses.end(), std::back_inserter(addresses));
				for(std::list<vaddress>::iterator adr_iter = addresses.begin(); adr_iter != addresses.end(); adr_iter++)
				{


						_iface_adr_map[*(iface_iter)].insert(*adr_iter);
						action = LinkEvent::ACTION_ADDRESS_ADDED;
						IBRCOMMON_LOGGER_DEBUG_TAG("LinkMonitor", 5) << "address ADDED:" << (*adr_iter).address() << " on interface " << (*iface_iter).toString() <<  IBRCOMMON_LOGGER_ENDL;
						LinkEvent lme(action, *(iface_iter), *(adr_iter));;
						_lm->raiseEvent(lme);
				}
			}

			try {
				//cond.wait(dtn::daemon::Configuration::getInstance().getNetwork().getLinkRequestInterval());
				cond.wait(_lm->getLinkRequestInterval());
			} catch (const Conditional::ConditionalAbortException &e){
				//reached, if conditional timed out
			}
		}
	}

	void LinkMonitor::__cancellation() throw()
	{
			IBRCOMMON_LOGGER_DEBUG_TAG("LinkRequester", 1) << "LinkRequester CANCELLED" << IBRCOMMON_LOGGER_ENDL;
			_running = false;
	}
}

