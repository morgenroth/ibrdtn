/*
 * StatisticLogger.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "StatisticLogger.h"
#include "core/NodeEvent.h"
#include "core/BundleEvent.h"
#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/Logger.h>
#include <sstream>
#include <typeinfo>
#include <ctime>

namespace dtn
{
	namespace daemon
	{
		StatisticLogger::StatisticLogger(LoggerType type, unsigned int interval, std::string address, unsigned int port)
		 : _timer(*this), _type(type), _interval(interval), _sentbundles(0), _recvbundles(0),
		   _core(dtn::core::BundleCore::getInstance()), _sock(NULL), _address(address), _port(port)
		{
			IBRCOMMON_LOGGER(info) << "Logging module initialized. mode = " << _type << ", interval = " << interval << IBRCOMMON_LOGGER_ENDL;
		}

		StatisticLogger::StatisticLogger(LoggerType type, unsigned int interval, ibrcommon::File file)
		 : _timer(*this), _file(file), _type(type), _interval(interval), _sentbundles(0), _recvbundles(0),
		   _core(dtn::core::BundleCore::getInstance()), _sock(NULL), _address("127.0.0.1"), _port(1234)
		{
		}

		StatisticLogger::~StatisticLogger()
		{

		}

		void StatisticLogger::componentUp() throw ()
		{
			if ((_type == LOGGER_FILE_PLAIN) || (_type == LOGGER_FILE_CSV))
			{
				// open statistic file
				if (_file.exists())
				{
					_fileout.open(_file.getPath().c_str(), ios_base::app);
				}
				else
				{
					_fileout.open(_file.getPath().c_str(), ios_base::trunc);
				}
			}

			// start the timer
			_timer.set(1);

			try {
				_timer.start();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER(error) << "failed to start StatisticLogger\n" << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}

			// register at the events
			bindEvent(dtn::core::NodeEvent::className);
			bindEvent(dtn::core::BundleEvent::className);

			if (_type == LOGGER_UDP)
			{
				_sock = new ibrcommon::udpsocket();
				_sock->up();
			}
		}

		size_t StatisticLogger::timeout(ibrcommon::Timer*)
		{
			switch (_type)
			{
			case LOGGER_STDOUT:
				writeStdLog(std::cout);
				break;

			case LOGGER_SYSLOG:
				writeSyslog(std::cout);
				break;

			case LOGGER_FILE_PLAIN:
				writePlainLog(_fileout);
				break;

			case LOGGER_FILE_CSV:
				writeCsvLog(_fileout);
				break;

			case LOGGER_FILE_STAT:
				writeStatLog();
				break;

			case LOGGER_UDP:
				writeUDPLog(*_sock);
				break;
			}

			return _interval;
		}

		void StatisticLogger::componentDown() throw ()
		{
			unbindEvent(dtn::core::NodeEvent::className);
			unbindEvent(dtn::core::BundleEvent::className);

			_timer.pause();

			if (_type >= LOGGER_FILE_PLAIN)
			{
				// close the statistic file
				_fileout.close();
			}

			if (_type == LOGGER_UDP)
			{
				_sock->down();
				delete _sock;
			}
		}

		void StatisticLogger::raiseEvent(const dtn::core::Event *evt)
		{
			try {
				const dtn::core::NodeEvent &node = dynamic_cast<const dtn::core::NodeEvent&>(*evt);

				// do not announce on node info updated
				if (node.getAction() == dtn::core::NODE_INFO_UPDATED) return;
			} catch (const std::bad_cast&) {
			}

			try {
				const dtn::core::BundleEvent &bundle = dynamic_cast<const dtn::core::BundleEvent&>(*evt);

				switch (bundle.getAction())
				{
					case dtn::core::BUNDLE_RECEIVED:
						_recvbundles++;
						break;

					case dtn::core::BUNDLE_FORWARDED:
						_sentbundles++;
						break;

					case dtn::core::BUNDLE_DELIVERED:
						_sentbundles++;
						break;

					default:
						return;
						break;
				}
			} catch (const std::bad_cast&) {
			}

			if ((_type == LOGGER_UDP) && (_sock != NULL))
			{
				writeUDPLog(*_sock);
			}
		}

		void StatisticLogger::writeSyslog(std::ostream &stream)
		{
			const std::set<dtn::core::Node> neighbors = _core.getNeighbors();
			dtn::storage::BundleStorage &storage = _core.getStorage();

			stream	<< "DTN-Stats: CurNeighbors " << neighbors.size()
					<< "; RecvBundles " << _recvbundles
					<< "; SentBundles " << _sentbundles
					<< "; StoredBundles " << storage.count()
					<< ";" << std::endl;
		}

		void StatisticLogger::writeStdLog(std::ostream &stream)
		{
			const std::set<dtn::core::Node> neighbors = _core.getNeighbors();
			size_t timestamp = dtn::utils::Clock::getTime();
			dtn::storage::BundleStorage &storage = _core.getStorage();

			stream	<< "Timestamp " << timestamp
					<< "; CurNeighbors " << neighbors.size()
					<< "; RecvBundles " << _recvbundles
					<< "; SentBundles " << _sentbundles
					<< "; StoredBundles " << storage.count()
					<< ";" << std::endl;
		}

		void StatisticLogger::writePlainLog(std::ostream &stream)
		{
			const std::set<dtn::core::Node> neighbors = _core.getNeighbors();
			size_t timestamp = dtn::utils::Clock::getTime();
			dtn::storage::BundleStorage &storage = _core.getStorage();

			stream 	<< timestamp << " "
					<< neighbors.size() << " "
					<< _recvbundles << " "
					<< _sentbundles << " "
					<< storage.count() << std::endl;
		}

		void StatisticLogger::writeCsvLog(std::ostream &stream)
		{
			const std::set<dtn::core::Node> neighbors = _core.getNeighbors();
			size_t timestamp = dtn::utils::Clock::getTime();
			dtn::storage::BundleStorage &storage = _core.getStorage();

			stream 	<< timestamp << ","
					<< neighbors.size() << ","
					<< _recvbundles << ","
					<< _sentbundles << ","
					<< storage.count() << std::endl;
		}

		void StatisticLogger::writeStatLog()
		{
			// open statistic file
			_fileout.open(_file.getPath().c_str(), ios_base::trunc);

			const std::set<dtn::core::Node> neighbors = _core.getNeighbors();
			size_t timestamp = dtn::utils::Clock::getTime();
			dtn::storage::BundleStorage &storage = _core.getStorage();

			time_t time_now = time(NULL);
			struct tm * timeinfo;
			timeinfo = localtime (&time_now);
			std::string datetime = asctime(timeinfo);
			datetime = datetime.substr(0, datetime.length() -1);

			_fileout << "IBR-DTN statistics" << std::endl;
			_fileout << std::endl;
			_fileout << "dtn timestamp: " << timestamp << " (" << datetime << ")" << std::endl;
			_fileout << "bundles sent: " << _sentbundles << std::endl;
			_fileout << "bundles received: " << _recvbundles << std::endl;
			_fileout << std::endl;
			_fileout << "bundles in storage: " << storage.count() << std::endl;
			_fileout << std::endl;
			_fileout << "neighbors (" << neighbors.size() << "):" << std::endl;

			for (std::set<dtn::core::Node>::const_iterator iter = neighbors.begin(); iter != neighbors.end(); iter++)
			{
				_fileout << "\t" << (*iter).toString() << std::endl;
			}

			_fileout << std::endl;

			// close the file
			_fileout.close();
		}

		void StatisticLogger::writeUDPLog(ibrcommon::udpsocket &socket)
		{
			std::stringstream ss;

			ss << dtn::core::BundleCore::local.getString() << "\n";

			writeCsvLog(ss);

			const std::set<dtn::core::Node> neighbors = _core.getNeighbors();
			for (std::set<dtn::core::Node>::const_iterator iter = neighbors.begin(); iter != neighbors.end(); iter++)
			{
				ss << (*iter).toString() << "\n";
			}

			ss << std::flush;

			std::string data = ss.str();
			ibrcommon::vaddress addr(_address, _port);
			socket.sendto(data.c_str(), data.length(), 0, addr);
		}

		const std::string StatisticLogger::getName() const
		{
			return "StatisticLogger";
		}
	}
}
