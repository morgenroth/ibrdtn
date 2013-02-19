/*
 * dtntunnel.cpp
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#include "config.h"

#include <iostream>
#include <stdlib.h>

// Base for send and receive bundle to/from the IBR-DTN daemon.
#include "ibrdtn/api/Client.h"

// Container for bundles.
#include "ibrdtn/api/Bundle.h"
#include "ibrdtn/api/BLOBBundle.h"

// Container for bundles carrying strings.
#include "ibrdtn/api/StringBundle.h"

//  TCP client implemented as a stream.
#include <ibrcommon/net/socket.h>

// Some classes to be thread-safe.
#include "ibrcommon/thread/Mutex.h"
#include "ibrcommon/thread/MutexLock.h"

// Basic functionalities for streaming.
#include <iostream>

// A queue for bundles.
#include <queue>

#include <csignal>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>

int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char clonedev[] = "/dev/net/tun";

  /* Arguments taken by the function:
   *
   * char *dev: the name of an interface (or '\0'). MUST have enough
   *   space to hold the interface name if '\0' is passed
   * int flags: interface flags (eg, IFF_TUN etc.)
   */

   /* open the clone device */
   if( (fd = open(clonedev, O_RDWR)) < 0 ) {
     return fd;
   }

   /* preparation of the struct ifr, of type "struct ifreq" */
   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

   if (*dev) {
     /* if a device name was specified, put it in the structure; otherwise,
      * the kernel will try to allocate the "next" device of the
      * specified type */
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
   }

   /* try to create the device */
   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
     close(fd);
     return err;
   }

  /* if the operation was successful, write back the name of the
   * interface to the variable "dev", so the caller can know
   * it. Note that the caller MUST reserve space in *dev (see calling
   * code below) */
  strcpy(dev, ifr.ifr_name);

  /* this is the special file descriptor that the caller will use to talk
   * with the virtual interface */
  return fd;
}

class TUN2BundleGateway : public dtn::api::Client
{
	public:
		TUN2BundleGateway(const std::string &app, ibrcommon::socketstream &stream, const std::string &ptp_dev)
		: dtn::api::Client(app, stream), _stream(stream), _fd(-1)
		{
			char tun_name[IFNAMSIZ];

			strcpy(tun_name, ptp_dev.c_str());
			_fd = tun_alloc(tun_name, IFF_TUN);  /* tun interface */

			tun_device = tun_name;

			if (_fd == -1)
			{
				std::cerr << "Error: failed to open tun device" << std::endl;
				return;
				// TODO: throw exception
			}

			// connect the API
			this->connect();
		}

		/**
		 * Destructor of the connection.
		 */
		virtual ~TUN2BundleGateway()
		{
			// Close the tcp connection.
			_stream.close();
		};

		void shutdown() {
			if (_fd < 0) return;
			// close socket
			::close(_fd);
			_fd = -1;
		}

		void process(const dtn::data::EID &endpoint) {
			char data[65536];
			int ret = ::read(_fd, data, sizeof(data));

			if (ret == -1) {
				// TODO: throw exception
				return;
			}

			// create a blob
			ibrcommon::BLOB::Reference blob = ibrcommon::BLOB::create();

			// add the data
			blob.iostream()->write(data, ret);

			// create a new bundle
			dtn::api::BLOBBundle b(endpoint, blob);

			// transmit the packet
			(*this) << b;
			flush();
		}

	private:
		ibrcommon::socketstream &_stream;

		// file descriptor for the tun device
		int _fd;

		std::string tun_device;

		/**
		 * In this API bundles are received asynchronous. To receive bundles it is necessary
		 * to overload the Client::received()-method. This will be call on a incoming bundles
		 * by another thread.
		 */
		void received(dtn::api::Bundle &b)
		{
			ibrcommon::BLOB::Reference ref = b.getData();
			ibrcommon::BLOB::iostream stream = ref.iostream();
			char data[65536];
			stream->read(data, sizeof(data));
			size_t ret = stream->gcount();
			if (::write(_fd, data, ret) < 0)
			{
				std::cerr << "error while writing" << std::endl;
			}
		}
};

bool m_running = true;
TUN2BundleGateway *_gateway = NULL;

void term(int signal)
{
	if (signal >= 1)
	{
		m_running = false;
		if (_gateway != NULL)
			_gateway->shutdown();
	}
}

void print_help(const char *argv0)
{
	cout << "-- dtntunnel (IBR-DTN) --" << endl;
	cout << "Syntax: " << argv0 << " [options] <name> <endpoint>" << endl;
	cout << " -d <dev>   Virtual network device to create (default: tun0)" << endl;
	cout << " -s <name>  Application suffix of the local endpoint (default: tunnel)" << endl;
}

int main(int argc, char *argv[])
{
	int index;
	int c;

	std::string ptp_dev("tun0");
	std::string app_name("tunnel");
	std::string endpoint("dtn:none");

	// catch process signals
	signal(SIGINT, term);
	signal(SIGTERM, term);

	while ((c = getopt (argc, argv, "d:")) != -1)
	switch (c)
	{
		case 'd':
			ptp_dev = optarg;
			break;

		case 's':
			app_name = optarg;
			break;

		default:
			print_help(argv[0]);
			return 1;
	}

	int optindex = 0;
	for (index = optind; index < argc; index++)
	{
		switch (optindex)
		{
		case 0:
			endpoint = std::string(argv[index]);
			break;
		}

		optindex++;
	}

	// print help if not enough parameters are set
	if (optindex < 1) { print_help(argv[0]); exit(0); }

	std::cout << "IBR-DTN IP <-> Bundle Tunnel" << std::endl;
	std::cout << "----------------------------" << std::endl;
	std::cout << "Local App. Suffix: " << app_name << std::endl;
	std::cout << "Peer: " << endpoint << std::endl;
	std::cout << "Tun device: " << ptp_dev << std::endl;
	std::cout << std::endl;

	// create a connection to the dtn daemon
	ibrcommon::vaddress addr("localhost", 4550);
	ibrcommon::socketstream conn(new ibrcommon::tcpsocket(addr));

	// set-up tun2bundle gateway
	TUN2BundleGateway gateway(app_name, conn, ptp_dev);
	_gateway = &gateway;

	std::cout << "Now you need to set-up the ip tunnel. You can use commands like this:" << std::endl;
	std::cout << "# sudo ip link set " << ptp_dev << " up" << std::endl;
	std::cout << "# sudo ip addr add 10.0.0.1/24 dev " << ptp_dev << std::endl;
	std::cout << std::endl;

	std::cout << "Tunnel up ...  " << std::flush;
	int display_state = 0;

	// destination
	dtn::data::EID eid(endpoint);

	while (m_running)
	{
		switch (display_state) {
		case 0:
			std::cout << "\b" << "-" << std::flush;
			display_state++;
			break;
		case 1:
			std::cout << "\b" << "\\" << std::flush;
			display_state++;
			break;
		case 2:
			std::cout << "\b" << "|" << std::flush;
			display_state++;
			break;
		case 3:
			std::cout << "\b" << "/" << std::flush;
			display_state = 0;
			break;
		default:
			display_state = 0;
			break;
		}
		gateway.process(eid);
	}

	gateway.shutdown();

	std::cout << std::endl;

	return 0;
}
