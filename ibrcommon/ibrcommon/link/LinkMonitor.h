/*
 * LinkRequester.h
 *
 *  Created on: Sep 9, 2013
 *      Author: goltzsch
 */

#ifndef LINKREQUESTER_H_
#define LINKREQUESTER_H_


#include <list>
#include "LinkManager.h"
#include "ibrcommon/net/vinterface.h"
#include "ibrcommon/net/vaddress.h"
#include "ibrcommon/thread/Thread.h"

namespace ibrcommon {

	class LinkMonitor : public ibrcommon::JoinableThread {

	public:
		LinkMonitor(LinkManager *lm, size_t link_request_interval);
		virtual ~LinkMonitor();

	protected:
		void run() throw ();
		void __cancellation() throw ();
	private:
		bool _running;
		LinkManager *_lm;

		std::map<vinterface,std::set<vaddress> > _iface_adr_map;
	};

}
#endif /* LINKREQUESTER_H_ */
