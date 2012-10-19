/*
 * vsocket.h
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

#ifndef VSOCKET_H_
#define VSOCKET_H_

#include <ibrcommon/data/File.h>
#include "ibrcommon/net/LinkManager.h"
#include <ibrcommon/net/socket.h>
#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/net/vaddress.h>
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/Thread.h>
#include <ibrcommon/thread/Queue.h>
#include <string>
#include <list>
#include <set>

namespace ibrcommon
{
	/**
	 * socketset is used for socket select calls and managing the socket instances
	 */
	typedef std::set<basesocket*> socketset;

	class vsocket_timeout : public socket_exception
	{
	public:
		vsocket_timeout(string error) : socket_exception(error)
		{};
	};

	class vsocket_interrupt : public socket_exception
	{
	public:
		vsocket_interrupt(string error) : socket_exception(error)
		{};
	};

	class vsocket : public ibrcommon::LinkManager::EventCallback
	{
	public:
		/**
		 * Constructor
		 */
		vsocket();

		/**
		 * Destructor
		 */
		virtual ~vsocket();

		/**
		 * Add a socket to the list monitored sockets. This socket instance
		 * will be adapted and destroyed on the destruction of the vsocket instance.
		 * @param socket Socket object to add
		 */
		void add(basesocket *socket);

		/**
		 * Add a socket to the list monitored sockets. This socket instance
		 * will be adapted and destroyed on the destruction of the vsocket instance.
		 * The socket will be shutdown if the state of the given interface changes.
		 * @param socket Socket object to add
		 * @param iface The interface to connect this socket to
		 */
		void add(basesocket *socket, const vinterface &iface);

		/**
		 * Remove the socket from the socket list.
		 * @param socket Pointer to the socket in the list
		 */
		void remove(basesocket *socket);

		/**
		 * get the number of added sockets
		 * @return the size of the internal socketset
		 */
		size_t size() const;

		/**
		 * @return a copy of the socketset
		 */
		socketset getAll() const;

		/**
		 * delete all sockets in the socketset
		 */
		void destroy();

		/**
		 * clear the socketset
		 */
		void clear();

		/**
		 * Enable all sockets and turn into the up state.
		 */
		void up() throw (socket_exception);

		/**
		 * Disable all sockets and turn into the down state.
		 */
		void down() throw (socket_exception);

		/**
		 * Execute a select on all associated sockets.
		 * @param callback
		 * @param tv
		 */
		void select(socketset *readset, socketset *writeset, socketset *errorset, struct timeval *tv = NULL) throw (socket_exception);

		void eventNotify(const LinkManagerEvent &evt);

	private:
		class pipesocket : public basesocket
		{
		public:
			pipesocket();
			virtual ~pipesocket();
			virtual void up() throw (socket_exception);
			virtual void down() throw (socket_exception);

			void read(char *buf, size_t len) throw (socket_exception);
			void write(const char *buf, size_t len) throw (socket_exception);

			int getOutput() const throw (socket_exception);

		private:
			int _output_fd;
		};

		void interrupt();

		ibrcommon::Mutex _socket_lock;
		socketset _sockets;

		pipesocket _pipe;

		// if this flag is set all selects call
		// will be aborted
		bool _interrupt_flag;
	};
}

#endif /* VSOCKET_H_ */
