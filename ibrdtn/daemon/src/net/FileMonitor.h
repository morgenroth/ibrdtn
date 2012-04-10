/*
 * FileMonitor.h
 *
 *  Created on: 10.04.2012
 *      Author: morgenro
 */

#include "Component.h"
#include "core/Node.h"
#include <ibrcommon/data/File.h>
#include <ibrcommon/net/vsocket.h>
#include <map>

#ifndef FILEMONITOR_H_
#define FILEMONITOR_H_

namespace dtn
{
	namespace net
	{
		class FileMonitor : public dtn::daemon::IndependentComponent
		{
		public:
			FileMonitor();
			virtual ~FileMonitor();

			void watch(const ibrcommon::File &watch);

			const std::string getName() const;

		protected:
			void componentUp();
			void componentRun();
			void componentDown();

			void __cancellation();

		private:
			void scan();
			bool isActive(const ibrcommon::File &path) const;

			void adopt(const ibrcommon::File &path);

			ibrcommon::File _watch;
			ibrcommon::vsocket _socket;
			int _inotify_sock;
			typedef std::map<int, ibrcommon::File> watch_map;
			watch_map _watch_map;
			bool _running;

			std::map<ibrcommon::File, dtn::core::Node> _active_paths;

		};
	}
} /* namespace ibrcommon */
#endif /* FILEMONITOR_H_ */
