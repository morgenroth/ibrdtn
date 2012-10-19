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
		//ibrcommon::LinkManager::getInstance().unregisterAllEvents(this);

		_pipe.down();
	}

	void vsocket::add(basesocket *socket)
	{
		_sockets.insert(socket);
	}

	void vsocket::add(basesocket *socket, const vinterface &iface)
	{
		_sockets.insert(socket);
		_socket_map[iface].insert(socket);
	}

	void vsocket::remove(basesocket *socket)
	{
		_sockets.erase(socket);

		// search for the same socket in the map
		for (std::map<vinterface, socketset>::iterator iter = _socket_map.begin(); iter != _socket_map.end(); iter++)
		{
			socketset &set = (*iter).second;
			set.erase(socket);
		}
	}

	size_t vsocket::size() const
	{
		return _sockets.size();
	}

	void vsocket::clear()
	{
		_sockets.clear();
		_socket_map.clear();
	}

	void vsocket::destroy()
	{
		for (socketset::iterator iter = _sockets.begin(); iter != _sockets.end(); iter++)
		{
			basesocket *sock = (*iter);

			try {
				sock->down();
			} catch (const socket_exception&) { }

			delete sock;
		}
		_sockets.clear();
		_socket_map.clear();
	}

	socketset vsocket::getAll() const
	{
		return _sockets;
	}

	socketset vsocket::get(const vinterface &iface) const
	{
		std::map<vinterface, socketset>::const_iterator iter = _socket_map.find(iface);

		if (iter == _socket_map.end()) {
			socketset empty;
			return empty;
		}

		return (*iter).second;
	}

	void vsocket::up() throw (socket_exception) {
		for (socketset::iterator iter = _sockets.begin(); iter != _sockets.end(); iter++) {
			try {
				if (!(*iter)->ready()) (*iter)->up();
			} catch (const socket_exception&) {
				// rewind all previously up'ped sockets
				for (socketset::iterator riter = _sockets.begin(); riter != iter; riter++) {
					(*riter)->down();
				}
				throw;
			}
		}
	}

	void vsocket::down() throw ()
	{
		ibrcommon::MutexLock l(_socket_lock);
		for (socketset::iterator iter = _sockets.begin(); iter != _sockets.end(); iter++) {
			try {
				(*iter)->down();
			} catch (const socket_exception&) { }
		}

		interrupt();
	}

	void vsocket::eventNotify(const LinkEvent &evt)
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

	void vsocket::select(socketset *readset, socketset *writeset, socketset *errorset, struct timeval *tv) throw (socket_exception)
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
				for (socketset::iterator iter = _sockets.begin();
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
			for (socketset::iterator iter = _sockets.begin();
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
}
