/*
 * vsocket.cpp
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

#include "ibrcommon/config.h"
#include "ibrcommon/net/vsocket.h"
#include "ibrcommon/TimeMeasurement.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/Logger.h"

#include <algorithm>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <errno.h>
#include <sstream>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace ibrcommon
{
	vsocket::pipesocket::pipesocket()
	 : _output_fd(-1)
	{ }

	vsocket::pipesocket::~pipesocket()
	{
	}

	int vsocket::pipesocket::getOutput() const throw (socket_exception)
	{
		if (_state == SOCKET_DOWN) throw socket_exception("output fd not available");
		return _output_fd;
	}

	void vsocket::pipesocket::up() throw (socket_exception)
	{
		if (_state != SOCKET_DOWN)
			throw socket_exception("socket is already up");

		int pipe_fds[2];

		// create a pipe for interruption
		if (::pipe(pipe_fds) < 0)
		{
			IBRCOMMON_LOGGER(error) << "Error " << errno << " creating pipe" << IBRCOMMON_LOGGER_ENDL;
			throw socket_exception("failed to create pipe");
		}

		_fd = pipe_fds[0];
		_output_fd = pipe_fds[1];

		this->set_blocking_mode(false);
		this->set_blocking_mode(false, _output_fd);

		_state = SOCKET_UP;
	}

	void vsocket::pipesocket::down() throw (socket_exception)
	{
		if (_state != SOCKET_UP)
			throw socket_exception("socket is not up");

		this->close();
		::close(_output_fd);
		_state = SOCKET_DOWN;
	}

	void vsocket::pipesocket::read(char *buf, size_t len) throw (socket_exception)
	{
		int ret = ::read(this->fd(), buf, len);
		if (ret == -1)
			throw socket_exception("read error");
		if (ret == 0)
			throw socket_exception("end of file");
	}

	void vsocket::pipesocket::write(const char *buf, size_t len) throw (socket_exception)
	{
		int ret = ::write(_output_fd, buf, len);
		if (ret == -1)
			throw socket_exception("write error");
	}


	vsocket::vsocket()
	 : _interrupt_flag(false)
	{
		_pipe.up();
	}

	vsocket::~vsocket()
	{
		ibrcommon::LinkManager::getInstance().unregisterAllEvents(this);

		_pipe.down();
	}

	void vsocket::add(basesocket *socket)
	{
		_sockets.insert(socket);
	}

	void vsocket::add(basesocket *socket, const vinterface &iface)
	{
		_sockets.insert(socket);
	}

	void vsocket::remove(basesocket *socket)
	{
		_sockets.erase(socket);
	}

//	int vsocket::bind(const vsocket::vbind &b)
//	{
//		_binds.push_back(b);
//		vsocket::vbind &vb = _binds.back();
//
//		try {
//			if (_options & VSOCKET_REUSEADDR) vb.set(VSOCKET_REUSEADDR);
//			if (_options & VSOCKET_BROADCAST) vb.set(VSOCKET_BROADCAST);
//			if (_options & VSOCKET_MULTICAST) vb.set(VSOCKET_MULTICAST);
//
//			vb.bind();
//
//			if (_options & VSOCKET_LINGER) vb.set(VSOCKET_LINGER);
//			if (_options & VSOCKET_NODELAY) vb.set(VSOCKET_NODELAY);
//			if (_options & VSOCKET_NONBLOCKING) vb.set(VSOCKET_NONBLOCKING);
//
//			// join all already assigned groups
//			for (mcast_groups::iterator iter = _groups.begin();
//					iter != _groups.end(); iter++) {
//				std::set<ibrcommon::vinterface> ifaces = (*iter).second;
//
//				for (std::set<ibrcommon::vinterface>::iterator it_iface = ifaces.begin();
//						it_iface != ifaces.end(); it_iface++) {
//					vb.join(iter->first, *it_iface);
//				}
//			}
//		} catch (const vsocket_exception&) {
//			_binds.pop_back();
//			throw;
//		}
//
//		return vb._fd;
//	}
//
//	std::set<int> vsocket::bind(const vinterface &iface, const int port, unsigned int socktype)
//	{
//		std::set<int> ret;
//		if (iface.empty()) { bind(port); return ret; }
//
//		// remember the port for dynamic bind/unbind
//		_portmap[iface] = port;
//		_typemap[iface] = socktype;
//
//		// watch at events on this interface
//		ibrcommon::LinkManager::getInstance().registerInterfaceEvent(iface, this);
//
//		// bind on all interfaces of "iface"!
//		const std::list<vaddress> addrlist = iface.getAddresses();
//
//		for (std::list<vaddress>::const_iterator iter = addrlist.begin(); iter != addrlist.end(); iter++)
//		{
//			if ((*iter).getFamily() == ibrcommon::vaddress::VADDRESS_INET6)
//				if ((_options & VSOCKET_LINKLOCAL) && ((*iter).getScope() != ibrcommon::vaddress::SCOPE_LINKLOCAL)) continue;
//
//			if (port == 0)
//			{
//				vsocket::vbind vb(iface, (*iter), socktype);
//				ret.insert( bind( vb ) );
//			}
//			else
//			{
//				vsocket::vbind vb(iface, (*iter), port, socktype);
//				ret.insert( bind( vb ) );
//			}
//		}
//
//		return ret;
//	}
//
//	void vsocket::unbind(const vinterface &iface, const int port)
//	{
//		// delete the watch at events on this interface
//		ibrcommon::LinkManager::getInstance().unregisterInterfaceEvent(iface, this);
//
//		// unbind all interfaces on interface "iface"!
//		const std::list<vaddress> addrlist = iface.getAddresses();
//
//		for (std::list<vaddress>::const_iterator iter = addrlist.begin(); iter != addrlist.end(); iter++)
//		{
//			if ((*iter).getFamily() == ibrcommon::vaddress::VADDRESS_INET6)
//				if ((_options & VSOCKET_LINKLOCAL) && ((*iter).getScope() != ibrcommon::vaddress::SCOPE_LINKLOCAL)) continue;
//			unbind( *iter, port );
//		}
//	}
//
//	std::set<int> vsocket::bind(const int port, unsigned int socktype)
//	{
//		std::set<int> ret;
//
//		vaddress addr6(vaddress::VADDRESS_INET6);
//		std::set<int> addr6_ret = bind( addr6, port, socktype );
//		ret.insert( addr6_ret.begin(), addr6_ret.end() );
//
//		vaddress addr(vaddress::VADDRESS_INET);
//		std::set<int> addr_ret = bind( addr, port, socktype );
//		ret.insert( addr_ret.begin(), addr_ret.end() );
//
//		return ret;
//	}
//
//	void vsocket::unbind(const int port)
//	{
//		vaddress addr6(vaddress::VADDRESS_INET6);
//		unbind( addr6, port );
//
//		vaddress addr(vaddress::VADDRESS_INET);
//		unbind( addr, port );
//	}
//
//	std::set<int> vsocket::bind(const vaddress &address, const int port, unsigned int socktype)
//	{
//		std::set<int> ret;
//		vsocket::vbind vb(address, port, socktype);
//		ret.insert( bind( vb ) );
//		return ret;
//	}
//
//	std::set<int> vsocket::bind(const ibrcommon::File &file, unsigned int socktype)
//	{
//		std::set<int> ret;
//		vsocket::vbind vb(file, socktype);
//		ret.insert( bind( vb ) );
//		return ret;
//	}
//
//	void vsocket::unbind(const vaddress &address, const int port)
//	{
//		for (std::list<vsocket::vbind>::iterator iter = _binds.begin(); iter != _binds.end(); iter++)
//		{
//			vsocket::vbind &b = (*iter);
//			if ((b._vaddress == address) && (b._port == port))
//			{
//				_unbind_queue.push(b);
//			}
//		}
//	}
//
//	void vsocket::unbind(const ibrcommon::File &file)
//	{
//		for (std::list<vsocket::vbind>::iterator iter = _binds.begin(); iter != _binds.end(); iter++)
//		{
//			vsocket::vbind &b = (*iter);
//			if (b._file == file)
//			{
//				_unbind_queue.push(b);
//			}
//		}
//	}
//
//	void vsocket::add(const int fd)
//	{
//		vsocket::vbind vb(fd);
//		bind(vb);
//	}
//
//	void vsocket::listen(int connections)
//	{
//		ibrcommon::MutexLock l(_bind_lock);
//		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
//				iter != _binds.end(); iter++)
//		{
//			ibrcommon::vsocket::vbind &bind = (*iter);
//			bind.listen(connections);
//		}
//
//		_listen_connections = connections;
//	}
//
//	void vsocket::relisten()
//	{
//		if (_listen_connections > 0)
//			listen(_listen_connections);
//	}
//
//	void vsocket::set(const Option &o)
//	{
//		// set options
//		_options |= o;
//
//		ibrcommon::MutexLock l(_bind_lock);
//		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
//				iter != _binds.end(); iter++)
//		{
//			ibrcommon::vsocket::vbind &bind = (*iter);
//			bind.set(o);
//		}
//	}
//
//	void vsocket::unset(const Option &o)
//	{
//		// unset options
//		_options &= ~(o);
//
//		ibrcommon::MutexLock l(_bind_lock);
//		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
//				iter != _binds.end(); iter++)
//		{
//			ibrcommon::vsocket::vbind &bind = (*iter);
//			bind.unset(o);
//		}
//	}
//
//	void vsocket::join(const ibrcommon::vaddress &group, const ibrcommon::vinterface &iface)
//	{
//		_groups[group].insert(iface);
//
//		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
//				iter != _binds.end(); iter++)
//		{
//			ibrcommon::vsocket::vbind &bind = (*iter);
//			bind.join(group, iface);
//		}
//	}
//
//	void vsocket::leave(const ibrcommon::vaddress &group)
//	{
//		const std::set<ibrcommon::vinterface> &ifaces = _groups[group];
//
//		for (std::set<ibrcommon::vinterface>::const_iterator it_ifaces = ifaces.begin();
//				it_ifaces != ifaces.end(); it_ifaces++) {
//			const ibrcommon::vinterface &iface = (*it_ifaces);
//
//			for (std::list<ibrcommon::vsocket::vbind>::iterator it_bind = _binds.begin();
//					it_bind != _binds.end(); it_bind++)
//			{
//				ibrcommon::vsocket::vbind &bind = (*it_bind);
//				bind.leave(group, iface);
//			}
//		}
//
//		_groups.erase(group);
//	}

	void vsocket::close()
	{
		ibrcommon::MutexLock l(_socket_lock);
		for (std::set<basesocket*>::iterator iter = _sockets.begin();
				iter != _sockets.end(); iter++)
		{
			basesocket *sock = (*iter);
			sock->down();
		}

		interrupt();
	}

	void vsocket::shutdown(int how)
	{
		ibrcommon::MutexLock l(_socket_lock);
		for (std::set<basesocket*>::iterator iter = _sockets.begin();
				iter != _sockets.end(); iter++)
		{
			basesocket *sock = (*iter);
			sock->shutdown(how);
		}

		interrupt();
	}

	void vsocket::eventNotify(const LinkManagerEvent &evt)
	{
//		const ibrcommon::vinterface &iface = evt.getInterface();
//		IBRCOMMON_LOGGER_DEBUG(5) << "update socket cause of event on interface " << iface.toString() << IBRCOMMON_LOGGER_ENDL;
//
//		// check if the portmap for this interface is available
//		if (_portmap.find(evt.getInterface()) == _portmap.end()) return;
//
//		try {
//			switch (evt.getType())
//			{
//			case LinkManagerEvent::EVENT_ADDRESS_ADDED:
//			{
//				IBRCOMMON_LOGGER_DEBUG(10) << "dynamic address bind on: " << evt.getAddress().toString() << ":" << _portmap[iface] << IBRCOMMON_LOGGER_ENDL;
//				if (_portmap[iface] == 0)
//				{
//					vsocket::vbind vb(iface, evt.getAddress(), _typemap[iface]);
//					bind( vb );
//				}
//				else
//				{
//					vsocket::vbind vb(iface, evt.getAddress(), _portmap[iface], _typemap[iface]);
//					bind( vb );
//				}
//				break;
//			}
//
//			case LinkManagerEvent::EVENT_ADDRESS_REMOVED:
//				IBRCOMMON_LOGGER_DEBUG(10) << "dynamic address unbind on: " << evt.getAddress().toString() << ":" << _portmap[iface] << IBRCOMMON_LOGGER_ENDL;
//				unbind(evt.getAddress(), _portmap[iface]);
//				break;
//
//			default:
//				break;
//			}
//		} catch (const ibrcommon::Exception &ex) {
//			IBRCOMMON_LOGGER(warning) << "dynamic bind process failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
//		}
//
//		// forward the event to the listen callback class
//		if (_cb != NULL) _cb->eventNotify(evt);
//
//		if (_send_only) {
//			// if this is send only socket, then process unbinds directly
//			process_unbind_queue();
//		} else {
//			// refresh the select call
//			refresh();
//		}
	}
//
//	void vsocket::setEventCallback(ibrcommon::LinkManager::EventCallback *cb)
//	{
//		_cb = cb;
//	}
//
//	const vsocket::fd_address_list vsocket::get(const ibrcommon::vinterface &iface)
//	{
//		fd_address_list ret;
//
//		ibrcommon::MutexLock l(_bind_lock);
//		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
//				iter != _binds.end(); iter++)
//		{
//			ibrcommon::vsocket::vbind &bind = (*iter);
//			if (bind._interface == iface)
//			{
//				ret.push_back( make_pair(bind._vaddress, bind._fd) );
//			}
//		}
//
//		return ret;
//	}
//
//	const vsocket::fd_address_list vsocket::get()
//	{
//		fd_address_list ret;
//
//		ibrcommon::MutexLock l(_bind_lock);
//		for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
//				iter != _binds.end(); iter++)
//		{
//			ibrcommon::vsocket::vbind &bind = (*iter);
//			ret.push_back( make_pair(bind._vaddress, bind._fd) );
//		}
//
//		return ret;
//	}
//
//	int vsocket::sendto(const void *buf, size_t n, const ibrcommon::vaddress &address, const unsigned int port)
//	{
//		try {
//			size_t ret = 0;
//			int flags = 0;
//
//			struct addrinfo hints, *ainfo;
//			memset(&hints, 0, sizeof hints);
//
//			hints.ai_socktype = SOCK_DGRAM;
//			ainfo = address.addrinfo(&hints, port);
//
//			ibrcommon::MutexLock l(_bind_lock);
//			for (std::list<ibrcommon::vsocket::vbind>::iterator iter = _binds.begin();
//					iter != _binds.end(); iter++)
//			{
//				ibrcommon::vsocket::vbind &bind = (*iter);
//				if (bind._vaddress.getFamily() == address.getFamily())
//				{
//					std::cout << "send to interface " << bind._interface.toString() << "; " << bind._vaddress.toString() << std::endl;
//					ret = ::sendto(bind._fd, buf, n, flags, ainfo->ai_addr, ainfo->ai_addrlen);
//				}
//			}
//
//			freeaddrinfo(ainfo);
//
//			return ret;
//		} catch (const vsocket_exception&) {
//			IBRCOMMON_LOGGER_DEBUG(5) << "can not send message to " << address.toString() << IBRCOMMON_LOGGER_ENDL;
//		}
//
//		return -1;
//	}
//
//	int recvfrom(int fd, char* data, size_t maxbuffer, std::string &address)
//	{
//		struct sockaddr_storage clientAddress;
//		socklen_t clientAddressLength = sizeof(clientAddress);
//
//		// data waiting
//		int ret = ::recvfrom(fd, data, maxbuffer, MSG_WAITALL, (struct sockaddr *) &clientAddress, &clientAddressLength);
//
//		char str[256];
//		::getnameinfo((struct sockaddr *) &clientAddress, clientAddressLength, str, 256, 0, 0, NI_NUMERICHOST);
//
//		address = std::string(str);
//
//		return ret;
//	}
//
//	void vsocket::refresh()
//	{
//		::write(_interrupt_pipe[1], "i", 1);
//	}
//
	void vsocket::interrupt()
	{
		_interrupt_flag = true;
		_pipe.write("i", 1);
	}

	void vsocket::select(std::set<basesocket*> *readset, std::set<basesocket*> *writeset, std::set<basesocket*> *errorset, struct timeval *tv) throw (socket_exception)
	{
		fd_set fds_read;
		fd_set fds_write;
		fd_set fds_error;

		int high_fd = 0;

		while (true)
		{
			FD_ZERO(&fds_read);
			FD_ZERO(&fds_write);
			FD_ZERO(&fds_error);

			// add the self-pipe-trick interrupt fd
			FD_SET(_pipe.fd(), &fds_read);
			high_fd = _pipe.fd();

			{
				ibrcommon::MutexLock l(_socket_lock);
				for (std::set<basesocket*>::iterator iter = _sockets.begin();
						iter != _sockets.end(); iter++)
				{
					basesocket &sock = (**iter);

					if (readset != NULL) {
						FD_SET(sock.fd(), &fds_read);
						if (high_fd < sock.fd()) high_fd = sock.fd();
					}

					if (writeset != NULL) {
						FD_SET(sock.fd(), &fds_write);
						if (high_fd < sock.fd()) high_fd = sock.fd();
					}

					if (errorset != NULL) {
						FD_SET(sock.fd(), &fds_error);
						if (high_fd < sock.fd()) high_fd = sock.fd();
					}
				}
			}

			// call the linux-like select with given timeout
			int res = __linux_select(high_fd + 1, &fds_read, &fds_write, &fds_error, tv);

			if (res < 0)
				throw socket_exception("select error");

			if (res == 0)
				throw vsocket_timeout("select timeout");

			if (FD_ISSET(_pipe.fd(), &fds_read))
			{
				IBRCOMMON_LOGGER_DEBUG(25) << "unblocked by self-pipe-trick" << IBRCOMMON_LOGGER_ENDL;

				// this was an interrupt with the self-pipe-trick
				char buf[2];
				_pipe.read(buf, 2);

//				// process the unbind queue
//				process_unbind_queue();
//
//				// listen on all new binds
//				relisten();

				// interrupt the method if requested
				if (_interrupt_flag)
				{
					// clear the abort flag
					_interrupt_flag = false;
					throw vsocket_interrupt("select interrupted");
				}

				// start over with the select call
				continue;
			}

			ibrcommon::MutexLock l(_socket_lock);
			for (std::set<basesocket*>::iterator iter = _sockets.begin();
					iter != _sockets.end(); iter++)
			{
				basesocket *sock = (*iter);

				if (readset != NULL) {
					if (FD_ISSET(sock->fd(), &fds_read))
					{
						readset->insert(sock);
					}
				}

				if (writeset != NULL) {
					if (FD_ISSET(sock->fd(), &fds_write))
					{
						writeset->insert(sock);
					}
				}

				if (errorset != NULL) {
					if (FD_ISSET(sock->fd(), &fds_error))
					{
						errorset->insert(sock);
					}
				}
			}

			break;
		}
	}

//	void vsocket::process_unbind_queue() {
//		if (!_unbind_queue.empty())
//		{
//			// unbind all removed sockets now
//			ibrcommon::Queue<vsocket::vbind>::Locked lq = _unbind_queue.exclusive();
//
//			vsocket::vbind &vb = lq.front();
//
//			for (std::list<vsocket::vbind>::iterator iter = _binds.begin(); iter != _binds.end(); iter++)
//			{
//				vsocket::vbind &i = (*iter);
//
//				if (i == vb)
//				{
//					IBRCOMMON_LOGGER_DEBUG(25) << "socket closed" << IBRCOMMON_LOGGER_ENDL;
//					i.close();
//					_binds.erase(iter);
//					break;
//				}
//			}
//
//			lq.pop();
//		}
//	}
//
//	vsocket::vbind::vbind(const int fd)
//	 : _type(BIND_CUSTOM), _vaddress(), _port(), _fd(fd)
//	{
//		// check for errors
//		if (_fd < 0) try {
//			check_socket_error( _fd );
//		} catch (const std::exception&) {
//			close();
//			throw;
//		}
//	}
//
//	vsocket::vbind::vbind(const vaddress &address, unsigned int socktype)
//	 : _type(BIND_ADDRESS_NOPORT), _vaddress(address), _port(0), _fd(0)
//	{
//		_fd = socket(address.getFamily(), socktype, 0);
//
//		// check for errors
//		if (_fd < 0) try {
//			check_socket_error( _fd );
//		} catch (const std::exception&) {
//			close();
//			throw;
//		}
//	}
//
//	vsocket::vbind::vbind(const vaddress &address, const int port, unsigned int socktype)
//	 : _type(BIND_ADDRESS), _vaddress(address), _port(port), _fd(0)
//	{
//		_fd = socket(address.getFamily(), socktype, 0);
//
//		// check for errors
//		if (_fd < 0) try {
//			check_socket_error( _fd );
//		} catch (const std::exception&) {
//			close();
//			throw;
//		}
//	}
//
//	vsocket::vbind::vbind(const ibrcommon::vinterface &iface, const vaddress &address, unsigned int socktype)
//	 : _type(BIND_ADDRESS_NOPORT), _vaddress(address), _port(0), _interface(iface), _fd(0)
//	{
//		_fd = socket(address.getFamily(), socktype, 0);
//
//		// check for errors
//		if (_fd < 0) try {
//			check_socket_error( _fd );
//		} catch (const std::exception&) {
//			close();
//			throw;
//		}
//	}
//
//	vsocket::vbind::vbind(const ibrcommon::vinterface &iface, const vaddress &address, const int port, unsigned int socktype)
//	 : _type(BIND_ADDRESS), _vaddress(address), _port(port), _interface(iface), _fd(0)
//	{
//		_fd = socket(address.getFamily(), socktype, 0);
//
//		// check for errors
//		if (_fd < 0) try {
//			check_socket_error( _fd );
//		} catch (const std::exception&) {
//			close();
//			throw;
//		}
//	}
//
//	vsocket::vbind::vbind(const ibrcommon::File &file, unsigned int socktype)
//	 : _type(BIND_FILE), _port(0), _file(file), _fd(0)
//	{
//		_fd = socket(AF_UNIX, socktype, 0);
//
//		// check for errors
//		if (_fd < 0) try {
//			check_socket_error( _fd );
//		} catch (const std::exception&) {
//			close();
//			throw;
//		}
//	}
//
//	vsocket::vbind::~vbind()
//	{
//	}
//
//	void vsocket::vbind::bind()
//	{
//		int bind_ret = 0;
//
//		switch (_type)
//		{
//			case BIND_CUSTOM:
//			{
//				// custom fd, do nothing
//				break;
//			}
//
//			case BIND_ADDRESS_NOPORT:
//			{
//				struct addrinfo hints, *res;
//				memset(&hints, 0, sizeof hints);
//
//				hints.ai_family = _vaddress.getFamily();
//				hints.ai_socktype = SOCK_STREAM;
//				hints.ai_flags = AI_PASSIVE;
//
//				res = _vaddress.addrinfo(&hints);
//				bind_ret = ::bind(_fd, res->ai_addr, res->ai_addrlen);
//				freeaddrinfo(res);
//				break;
//			}
//
//			case BIND_ADDRESS:
//			{
//				struct addrinfo hints, *res;
//				memset(&hints, 0, sizeof hints);
//
//				hints.ai_family = _vaddress.getFamily();
//				hints.ai_socktype = SOCK_STREAM;
//				hints.ai_flags = AI_PASSIVE;
//
//				res = _vaddress.addrinfo(&hints, _port);
//				bind_ret = ::bind(_fd, res->ai_addr, res->ai_addrlen);
//				freeaddrinfo(res);
//				break;
//			}
//
//			case BIND_FILE:
//			{
//				// remove old sockets
//				unlink(_file.getPath().c_str());
//
//				struct sockaddr_un address;
//				size_t address_length;
//
//				address.sun_family = AF_UNIX;
//				strcpy(address.sun_path, _file.getPath().c_str());
//				address_length = sizeof(address.sun_family) + strlen(address.sun_path);
//
//				// bind to the socket
//				bind_ret = ::bind(_fd, (struct sockaddr *) &address, address_length);
//
//				break;
//			}
//		}
//
//		if ( bind_ret < 0) check_bind_error( errno );
//	}

//	void vsocket::vbind::shutdown()
//	{
//		::shutdown(_fd, SHUT_RDWR);
//	}
//
//	bool vsocket::vbind::operator==(const int &fd) const
//	{
//		return (fd == _fd);
//	}
//
//	bool vsocket::vbind::operator==(const vbind &obj) const
//	{
//		if (obj._type != _type) return false;
//		if (obj._port != _port) return false;
//		if (obj._vaddress != _vaddress) return false;
//		if (obj._file.getPath() != _file.getPath()) return false;
//
//		return true;
//	}
}
