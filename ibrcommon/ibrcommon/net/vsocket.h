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
		 * Enable all sockets and turn into the up state.
		 */
		void up() throw (socket_exception);

		/**
		 * Disable all sockets and turn into the down state.
		 */
		void down() throw (socket_exception);

		/**
		 * close all sockets in this vsocket
		 */
		void close();
		void shutdown(int how);

		/**
		 * Execute a select on all associated sockets.
		 * @param callback
		 * @param tv
		 */
		void select(std::set<basesocket*> *readset, std::set<basesocket*> *writeset, std::set<basesocket*> *errorset, struct timeval *tv = NULL) throw (socket_exception);

//
//		std::set<int> bind(const vaddress &address, const int port, unsigned int socktype = SOCK_STREAM);
//		std::set<int> bind(const vinterface &iface, const int port, unsigned int socktype = SOCK_STREAM);
//		std::set<int> bind(const int port, unsigned int socktype = SOCK_STREAM);
//		std::set<int> bind(const ibrcommon::File &file, unsigned int socktype = SOCK_STREAM);
//
//		void unbind(const vaddress &address, const int port);
//		void unbind(const vinterface &iface, const int port);
//		void unbind(const int port);
//		void unbind(const ibrcommon::File &file);
//
//		void add(const int fd);
//
//		typedef std::pair<ibrcommon::vaddress, int> fd_address_entry;
//		typedef std::list<fd_address_entry> fd_address_list;
//
//		const fd_address_list get(const ibrcommon::vinterface &iface);
//		const fd_address_list get();
//
//		void listen(int connections);
//		void relisten();
//
//		void set(const Option &o);
//		void unset(const Option &o);
//
//		/**
//		 * Join a multicast group
//		 * @param group
//		 */
//		void join(const ibrcommon::vaddress &group, const ibrcommon::vinterface &iface);
//
//		/**
//		 * Leave a multicast group
//		 * @param group
//		 */
//		void leave(const ibrcommon::vaddress &group);
//
//		/**
//		 * close all fds
//		 */
//		void close();
//		void shutdown();
//
//		/**
//		 * return the first fd
//		 * @return
//		 */
//		int fd();
//
//		void select(std::list<int> &fds, struct timeval *tv = NULL);
//		int sendto(const void *buf, size_t n, const ibrcommon::vaddress &address, const unsigned int port);
//
		void eventNotify(const LinkManagerEvent &evt);
//
//		void setEventCallback(ibrcommon::LinkManager::EventCallback *cb);
//
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

//		class vbind
//		{
//		public:
//			enum bind_type
//			{
//				BIND_ADDRESS,
//				BIND_FILE,
//				BIND_CUSTOM,
//				BIND_ADDRESS_NOPORT
//			};
//
//			const bind_type _type;
//			const vaddress _vaddress;
//			const int _port;
//			const ibrcommon::File _file;
//			const ibrcommon::vinterface _interface;
//
//			int _fd;
//
//			vbind(int fd);
//			vbind(const vaddress &address, unsigned int socktype);
//			vbind(const vaddress &address, const int port, unsigned int socktype);
//			vbind(const ibrcommon::vinterface &iface, const vaddress &address, unsigned int socktype);
//			vbind(const ibrcommon::vinterface &iface, const vaddress &address, const int port, unsigned int socktype);
//			vbind(const ibrcommon::File &file, unsigned int socktype);
//			virtual ~vbind();
//
//			void bind();
//			void listen(int connections);
//
//			void set(const vsocket::Option &o);
//			void unset(const vsocket::Option &o);
//
//			/**
//			 * Join a multicast group
//			 * @param group
//			 */
//			void join(const ibrcommon::vaddress &group, const ibrcommon::vinterface &iface);
//
//			/**
//			 * Leave a multicast group
//			 * @param group
//			 */
//			void leave(const ibrcommon::vaddress &group, const ibrcommon::vinterface &iface);
//
//			void close();
//			void shutdown();
//
//			bool operator==(const vbind &obj) const;
//
//			/**
//			 * check if the given fd is part of this bind
//			 * @param fd
//			 * @return True if the fd matches.
//			 */
//			bool operator==(const int &fd) const;
//
//		private:
//			void check_socket_error(const int err) const;
//			void check_bind_error(const int err) const;
//		};
//
//		int bind(const vsocket::vbind &b);
//		void refresh();
		void interrupt();
//		void process_unbind_queue();
//
//		ibrcommon::Mutex _bind_lock;
//		std::list<vsocket::vbind> _binds;
//		std::map<vinterface, unsigned int> _portmap;
//		std::map<vinterface, unsigned int> _typemap;
//
//		// multicast groups
//		typedef std::map< ibrcommon::vaddress, std::set<ibrcommon::vinterface> > mcast_groups;
//		mcast_groups _groups;
//
//		unsigned int _options;
//		bool _interrupt;
//
		//bool rebind;


		ibrcommon::Mutex _socket_lock;
		std::set<basesocket*> _sockets;

		pipesocket _pipe;

		// if this flag is set all selects call
		// will be aborted
		bool _interrupt_flag;

//
//		ibrcommon::Queue<vbind> _unbind_queue;
//
//		int _listen_connections;
//		ibrcommon::LinkManager::EventCallback *_cb;
//
//		// if the socket is used to send only, this parameter should set to true
//		bool _send_only;
	};
//
//	int recvfrom(int fd, char* data, size_t maxbuffer, std::string &address);
}

#endif /* VSOCKET_H_ */
