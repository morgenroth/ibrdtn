/*
 * FileMonitor.cpp
 *
 *  Created on: 10.04.2012
 *      Author: morgenro
 */

#include "config.h"
#include "net/FileMonitor.h"
#include "core/BundleCore.h"
#include <ibrcommon/Logger.h>
#include <ibrcommon/data/ConfigFile.h>

#ifdef HAVE_SYS_INOTIFY_H
#include <sys/inotify.h>
#include <unistd.h>
#endif

namespace dtn
{
	namespace net
	{
		FileMonitor::FileMonitor()
		 : _inotify_sock(0), _running(true)
		{
#ifdef HAVE_SYS_INOTIFY_H
			_inotify_sock = inotify_init();
#endif
		}

		FileMonitor::~FileMonitor()
		{
			join();
		}

		void FileMonitor::watch(const ibrcommon::File &watch)
		{
			IBRCOMMON_LOGGER_DEBUG(5) << "watch on: " << watch.getPath() << IBRCOMMON_LOGGER_ENDL;

			if (!watch.isDirectory())
				throw ibrcommon::Exception("can not watch files, please specify a directory");

#ifdef HAVE_SYS_INOTIFY_H
			int wd = inotify_add_watch(_inotify_sock, watch.getPath().c_str(), IN_CREATE | IN_DELETE);
			_watch_map[wd] = watch;
#endif
		}

		void FileMonitor::componentUp()
		{
#ifdef HAVE_SYS_INOTIFY_H
			_socket.add(_inotify_sock);
#endif
		}

		void FileMonitor::componentRun()
		{
#ifdef HAVE_SYS_INOTIFY_H
			while (_running)
			{
				std::list<int> fds;

				// scan for mounts
				scan();

				try {
					// select on all bound sockets
					_socket.select(fds);

					unsigned char buf[1024];

					// receive from all sockets
					for (std::list<int>::const_iterator iter = fds.begin(); iter != fds.end(); iter++)
					{
						int fd = (*iter);
						::read(fd, &buf, 1024);
					}

					::sleep(2);
				} catch (const ibrcommon::vsocket_timeout&) { };
			}
#endif
		}

		void FileMonitor::componentDown()
		{
#ifdef HAVE_SYS_INOTIFY_H
			for (watch_map::iterator iter = _watch_map.begin(); iter != _watch_map.end(); iter++)
			{
				const int wd = (*iter).first;
				inotify_rm_watch(_inotify_sock, wd);
			}
#endif
			_watch_map.clear();
		}

		void FileMonitor::__cancellation()
		{
#ifdef HAVE_SYS_INOTIFY_H
			_socket.close();
#endif
		}

		void FileMonitor::scan()
		{
			IBRCOMMON_LOGGER_DEBUG(5) << "scan for file changes" << IBRCOMMON_LOGGER_ENDL;

			std::set<ibrcommon::File> watch_set;

			for (watch_map::iterator iter = _watch_map.begin(); iter != _watch_map.end(); iter++)
			{
				const ibrcommon::File &path = (*iter).second;
				std::list<ibrcommon::File> files;

				if (path.getFiles(files) == 0)
				{
					for (std::list<ibrcommon::File>::iterator iter = files.begin(); iter != files.end(); iter++)
					{
						watch_set.insert(*iter);
					}
				}
				else
				{
					IBRCOMMON_LOGGER(error) << "scan of " << path.getPath() << " failed" << IBRCOMMON_LOGGER_ENDL;
				}
			}

			// check for new directories
			for (std::set<ibrcommon::File>::iterator iter = watch_set.begin(); iter != watch_set.end(); iter++)
			{
				const ibrcommon::File &path = (*iter);
				if (path.isDirectory() && !path.isSystem())
				{
					if (!isActive(path)) {
						adopt(path);
					}
				}
			}

			// look for gone directories
			for (std::map<ibrcommon::File, dtn::core::Node>::iterator iter = _active_paths.begin(); iter != _active_paths.end();)
			{
				const ibrcommon::File &path = (*iter).first;
				const dtn::core::Node &node = (*iter).second;

				if (watch_set.find(path) == watch_set.end())
				{
					dtn::core::BundleCore::getInstance().removeConnection(node);
					IBRCOMMON_LOGGER_DEBUG(5) << "Node on drive gone: " << node.getEID().getString() << IBRCOMMON_LOGGER_ENDL;
					_active_paths.erase(iter++);
				}
				else
				{
					iter++;
				}
			}
		}

		void FileMonitor::adopt(const ibrcommon::File &path)
		{
			// look for ".dtndrive" file
			const ibrcommon::File drivefile = path.get(".dtndrive");
			if (drivefile.exists())
			{
				ibrcommon::ConfigFile cf(drivefile.getPath());

				try {
					const dtn::data::EID eid( cf.read<std::string>("EID") );

					dtn::core::Node n(eid);
					const std::string uri = "file://" + path.getPath() + "/" + cf.read<std::string>("PATH");

					n.add(dtn::core::Node::URI(dtn::core::Node::NODE_STATIC, dtn::core::Node::CONN_FILE, uri, 0, 10));
					dtn::core::BundleCore::getInstance().addConnection(n);

					_active_paths[path] = n;

					IBRCOMMON_LOGGER_DEBUG(5) << "Node on drive found: " << n.getEID().getString() << IBRCOMMON_LOGGER_ENDL;

				} catch (const ibrcommon::ConfigFile::key_not_found&) {};
			}
		}

		bool FileMonitor::isActive(const ibrcommon::File &path) const
		{
			return (_active_paths.find(path) != _active_paths.end());
		}

		const std::string FileMonitor::getName() const
		{
			return "FileMonitor";
		}
	}
} /* namespace ibrcommon */
