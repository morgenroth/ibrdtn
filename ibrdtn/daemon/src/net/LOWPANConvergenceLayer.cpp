#include "net/LOWPANConvergenceLayer.h"
#include "net/LOWPANConnection.h"
#include "core/BundleCore.h"
#include "core/TimeEvent.h"
#include <ibrcommon/net/lowpanstream.h>
#include <ibrcommon/net/UnicastSocketLowpan.h>
#include <ibrcommon/net/lowpansocket.h>

#include <ibrcommon/Logger.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/TimeMeasurement.h>

#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <list>

#define EXTENDED_MASK	0x08
#define SEQ_NUM_MASK	0x07

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		LOWPANConvergenceLayer::LOWPANConvergenceLayer(ibrcommon::vinterface net, int panid, unsigned int mtu)
			: DiscoveryAgent(dtn::daemon::Configuration::getInstance().getDiscovery()),
			  _socket(NULL), _net(net), _panid(panid), _ipnd_buf(new char[BUFF_SIZE+2]), _ipnd_buf_len(0), m_maxmsgsize(mtu), _running(false)
		{
			_socket = new ibrcommon::UnicastSocketLowpan();
		}

		LOWPANConvergenceLayer::~LOWPANConvergenceLayer()
		{
			componentDown();
			delete _socket;
		}

		dtn::core::Node::Protocol LOWPANConvergenceLayer::getDiscoveryProtocol() const
		{
			return dtn::core::Node::CONN_LOWPAN;
		}

		void LOWPANConvergenceLayer::update(const ibrcommon::vinterface &iface, std::string &name, std::string &params) throw(dtn::net::DiscoveryServiceProvider::NoServiceHereException)
		{
			if (iface == _net)
			{
				name = "lowpancl";
				stringstream service;

				struct sockaddr_ieee802154 address;
				address.addr.addr_type = IEEE802154_ADDR_SHORT;
				address.addr.pan_id = _panid;

				// Get address via netlink
				ibrcommon::lowpansocket::getAddress(&address.addr, iface);

				//FIXME better naming for address and panid. This will need updates to the service parser.
				service << "ip=" << address.addr.short_addr << ";port=" << _panid << ";";

				params = service.str();
			}
			else
			{
				 throw dtn::net::DiscoveryServiceProvider::NoServiceHereException();
			}
		}

		void LOWPANConvergenceLayer::send_cb(char *buf, int len, unsigned int address)
		{
			// Add own address at the end
			struct sockaddr_ieee802154 _sockaddr;
			unsigned short local_addr;

			_socket->getAddress(&_sockaddr.addr, _net);

			local_addr = _sockaddr.addr.short_addr;

			// get a lowpan peer
			ibrcommon::lowpansocket::peer p = _socket->getPeer(address, _sockaddr.addr.pan_id);
			if (len > 113)
				IBRCOMMON_LOGGER(error) << "LOWPANConvergenceLayer::send_cb buffer to big to be transferred (" << len << ")."<< IBRCOMMON_LOGGER_ENDL;

			// Add own address at the end
			memcpy(&buf[len], &local_addr, 2);

			// set write lock
			ibrcommon::MutexLock l(m_writelock);

			// send converted line
			int ret = p.send(buf, len + 2);

			if (ret == -1)
			{
				// CL is busy
				throw(ibrcommon::Exception("Send on socket failed"));
			}
		}

		void LOWPANConvergenceLayer::queue(const dtn::core::Node &node, const ConvergenceLayer::Job &job)
		{
			const std::list<dtn::core::Node::URI> uri_list = node.get(dtn::core::Node::CONN_LOWPAN);
			if (uri_list.empty()) return;

			const dtn::core::Node::URI &uri = uri_list.front();

			std::string address;
			unsigned int pan;

			uri.decode(address, pan);

			IBRCOMMON_LOGGER_DEBUG(10) << "LOWPANConvergenceLayer::queue"<< IBRCOMMON_LOGGER_ENDL;

			ibrcommon::MutexLock lc(_connection_lock);
			getConnection(atoi(address.c_str()))->_sender.queue(job);
		}

		LOWPANConnection* LOWPANConvergenceLayer::getConnection(unsigned short address)
		{
			// Test if connection for this address already exist
			std::list<LOWPANConnection*>::iterator i;
			for(i = ConnectionList.begin(); i != ConnectionList.end(); ++i)
			{
				IBRCOMMON_LOGGER_DEBUG(10) << "Connection address: " << hex << (*i)->_address << IBRCOMMON_LOGGER_ENDL;
				if ((*i)->_address == address)
					return (*i);
			}

			// Connection does not exist, create one and put it into the list
			LOWPANConnection *connection = new LOWPANConnection(address, (*this));

			ConnectionList.push_back(connection);
			IBRCOMMON_LOGGER_DEBUG(10) << "LOWPANConvergenceLayer::getConnection "<< connection->_address << IBRCOMMON_LOGGER_ENDL;
			connection->start();
			return connection;
		}

		void LOWPANConvergenceLayer::remove(const LOWPANConnection *conn)
		{
			ibrcommon::MutexLock lc(_connection_lock);

			std::list<LOWPANConnection*>::iterator i;
			for(i = ConnectionList.begin(); i != ConnectionList.end(); ++i)
			{
				if ((*i) == conn)
				{
					ConnectionList.erase(i);
					return;
				}
			}
		}

		void LOWPANConvergenceLayer::componentUp()
		{
			bindEvent(dtn::core::TimeEvent::className);
			try {
				try {
					ibrcommon::UnicastSocketLowpan &sock = dynamic_cast<ibrcommon::UnicastSocketLowpan&>(*_socket);
					sock.bind(_panid, _net);

					addService(this);
				} catch (const std::bad_cast&) {

				}
			} catch (const ibrcommon::lowpansocket::SocketException &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "Failed to add LOWPAN ConvergenceLayer on " << _net.toString() << ":" << _panid << IBRCOMMON_LOGGER_ENDL;
				IBRCOMMON_LOGGER_DEBUG(10) << "Exception: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void LOWPANConvergenceLayer::componentDown()
		{
			unbindEvent(dtn::core::TimeEvent::className);
			stop();
			join();
		}

		void LOWPANConvergenceLayer::sendAnnoucement(const u_int16_t &sn, std::list<dtn::net::DiscoveryService> &services)
		{
			IBRCOMMON_LOGGER_DEBUG(10) << "LOWPAN IPND beacon send started" << IBRCOMMON_LOGGER_ENDL;

			DiscoveryAnnouncement announcement(DiscoveryAnnouncement::DISCO_VERSION_01, dtn::core::BundleCore::local);

			// set sequencenumber
			announcement.setSequencenumber(sn);

			// clear all services
			announcement.clearServices();

			// add services
			for (std::list<DiscoveryService>::iterator iter = services.begin(); iter != services.end(); iter++)
			{
				DiscoveryService &service = (*iter);

				try {
					// update service information
					service.update(_net);

					// add service to discovery message
					announcement.addService(service);
				} catch (const dtn::net::DiscoveryServiceProvider::NoServiceHereException&) {

				}
			}
			// Set extended header bit. Everything else 0
			_ipnd_buf[0] =  0x08;
			// Set discovery bit in extended header
			_ipnd_buf[1] = 0x80;

			// serialize announcement
			stringstream ss;
			ss << announcement;

			int len = ss.str().size();
			if (len > 111)
				IBRCOMMON_LOGGER(error) << "Discovery announcement to big (" << len << ")" << IBRCOMMON_LOGGER_ENDL;

			memcpy(_ipnd_buf+2, ss.str().c_str(), len);

			send_cb(_ipnd_buf, len + 2, 0xffff);
		}

		void LOWPANConvergenceLayer::componentRun()
		{
			_running = true;

			while (_running)
			{
				ibrcommon::MutexLock l(m_readlock);

				char data[m_maxmsgsize];
				char header;
				unsigned short address;

				IBRCOMMON_LOGGER_DEBUG(10) << "LOWPANConvergenceLayer::componentRun early" << IBRCOMMON_LOGGER_ENDL;

				// Receive full frame from socket
				int len = _socket->receive(data, m_maxmsgsize);

				IBRCOMMON_LOGGER_DEBUG(10) << "LOWPANConvergenceLayer::componentRun" << IBRCOMMON_LOGGER_ENDL;

				// We got nothing from the socket, keep reading
				if (len <= 0)
					continue;

				// Retrieve header of frame
				header = data[0];

				// Retrieve sender address from the end of the frame
				address = ((char)data[len-1] << 8) | data[len-2];

				// Check for extended header and retrieve if available
				if ((header & EXTENDED_MASK) && (data[1] & 0x80)) {
					IBRCOMMON_LOGGER_DEBUG(10) << "Received announcement for LoWPAN discovery: ADDRESS " << address << IBRCOMMON_LOGGER_ENDL;
					DiscoveryAnnouncement announce;
					stringstream ss;
					ss.write(data+2, len-4);
					ss >> announce;
					DiscoveryAgent::received(announce, 30);
					continue;
				}

				ibrcommon::MutexLock lc(_connection_lock);

				// Connection instance for this address
				LOWPANConnection* connection = getConnection(address);

				// Decide in which queue to write based on the src address
				connection->getStream().queue(data, len-2); // Cut off address from end

				yield();
			}
		}

		void LOWPANConvergenceLayer::raiseEvent(const Event *evt)
		{
			try {
				const TimeEvent &time=dynamic_cast<const TimeEvent&>(*evt);
				if (time.getAction() == TIME_SECOND_TICK)
					if (time.getTimestamp()%5 == 0)
						timeout();
			} catch (const std::bad_cast&)
			{}
		}

		void LOWPANConvergenceLayer::__cancellation()
		{
			_running = false;
			_socket->shutdown();
		}

		const std::string LOWPANConvergenceLayer::getName() const
		{
			return "LOWPANConvergenceLayer";
		}
	}
}
