/*
 * vsocket.h
 *
 *  Created on: 14.12.2010
 *      Author: morgenro
 */

#ifndef VSOCKET_H_
#define VSOCKET_H_

#include <ibrcommon/data/File.h>
#include "ibrcommon/net/LinkManager.h"
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
	 * This select emulated the linux behavior of a select.
	 * It measures the time being in the select call and decrement the given timeout value.
	 * @param nfds
	 * @param readfds
	 * @param writefds
	 * @param exceptfds
	 * @param timeout
	 * @return
	 */
	int __nonlinux_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

	class vsocket_exception : public Exception
	{
	public:
		vsocket_exception(string error) : Exception(error)
		{};
	};

	class vsocket_timeout : public vsocket_exception
	{
	public:
		vsocket_timeout(string error) : vsocket_exception(error)
		{};
	};

	class vsocket_interrupt : public vsocket_exception
	{
	public:
		vsocket_interrupt(string error) : vsocket_exception(error)
		{};
	};

	class vsocket : public ibrcommon::LinkManager::EventCallback
	{
	public:
		enum Option
		{
			VSOCKET_REUSEADDR = 1 << 0,
			VSOCKET_LINGER = 1 << 1,
			VSOCKET_NODELAY = 1 << 2,
			VSOCKET_BROADCAST = 1 << 3,
			VSOCKET_NONBLOCKING = 1 << 4,
			VSOCKET_MULTICAST = 1 << 5,
			VSOCKET_MULTICAST_V6 = 1 << 6
		};

		static void set_non_blocking(int socket, bool nonblock = true);

		vsocket();
		virtual ~vsocket();

		int bind(const vaddress &address, const int port, unsigned int socktype = SOCK_STREAM);
		void bind(const vinterface &iface, const int port, unsigned int socktype = SOCK_STREAM);
		int bind(const int port, unsigned int socktype = SOCK_STREAM);
		int bind(const ibrcommon::File &file, unsigned int socktype = SOCK_STREAM);

		void unbind(const vaddress &address, const int port);
		void unbind(const vinterface &iface, const int port);
		void unbind(const int port);
		void unbind(const ibrcommon::File &file);

		void add(const int fd);

		const std::list<int> get(const ibrcommon::vinterface &iface, const ibrcommon::vaddress::Family f = vaddress::VADDRESS_UNSPEC);
		const std::list<int> get(const ibrcommon::vaddress::Family f = vaddress::VADDRESS_UNSPEC);

		void listen(int connections);
		void relisten();

		void set(const Option &o);
		void unset(const Option &o);

		void close();
		void shutdown();

		int fd();

		void select(std::list<int> &fds, struct timeval *tv = NULL);
		int sendto(const void *buf, size_t n, const ibrcommon::vaddress &address, const unsigned int port);

		void eventNotify(const LinkManagerEvent &evt);

		void setEventCallback(ibrcommon::LinkManager::EventCallback *cb);

	private:
		class vbind
		{
		public:
			enum bind_type
			{
				BIND_ADDRESS,
				BIND_FILE,
				BIND_CUSTOM,
				BIND_ADDRESS_NOPORT
			};

			const bind_type _type;
			const vaddress _vaddress;
			const int _port;
			const ibrcommon::File _file;
			const ibrcommon::vinterface _interface;

			int _fd;

			vbind(int fd);
			vbind(const vaddress &address, unsigned int socktype);
			vbind(const vaddress &address, const int port, unsigned int socktype);
			vbind(const ibrcommon::vinterface &iface, const vaddress &address, unsigned int socktype);
			vbind(const ibrcommon::vinterface &iface, const vaddress &address, const int port, unsigned int socktype);
			vbind(const ibrcommon::File &file, unsigned int socktype);
			virtual ~vbind();

			void bind();
			void listen(int connections);

			void set(const vsocket::Option &o);
			void unset(const vsocket::Option &o);

			void close();
			void shutdown();

			bool operator==(const vbind &obj) const;

		private:
			void check_socket_error(const int err) const;
			void check_bind_error(const int err) const;
		};

		int bind(const vsocket::vbind &b);
		void refresh();
		void interrupt();

		ibrcommon::Mutex _bind_lock;
		std::list<vsocket::vbind> _binds;
		std::map<vinterface, unsigned int> _portmap;
		std::map<vinterface, unsigned int> _typemap;

		unsigned int _options;
		bool _interrupt;

		//bool rebind;
		int _interrupt_pipe[2];

		ibrcommon::Queue<vbind> _unbind_queue;

		int _listen_connections;
		ibrcommon::LinkManager::EventCallback *_cb;
	};

	int recvfrom(int fd, char* data, size_t maxbuffer, std::string &address);
}

#endif /* VSOCKET_H_ */
