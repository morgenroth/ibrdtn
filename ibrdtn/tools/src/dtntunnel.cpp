/*
 * dtntunnel.cpp
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

//============================================================================
// Name        : IPtunnel.cpp
// Author      : IBR, TU Braunschweig
// Version     :
// Copyright   :
// Description : Hello World in C++, Ansi-style
//============================================================================

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

using namespace std;

/*
 * Allocate TUN device, returns opened fd.
 * Stores dev name in the first arg(must be large enough).
 */
static int tun_open_common0(char *dev, int istun)
{
    char tunname[14];
    int i, fd, err;

    if( *dev ) {
       sprintf(tunname, "/dev/%s", dev);
       return open(tunname, O_RDWR);
    }

    sprintf(tunname, "/dev/%s", istun ? "tun" : "tap");
    err = 0;
    for(i=0; i < 255; i++){
       sprintf(tunname + 8, "%d", i);
       /* Open device */
       if( (fd=open(tunname, O_RDWR)) > 0 ) {
          strcpy(dev, tunname + 5);
          return fd;
       }
       else if (errno != ENOENT)
          err = errno;
       else if (i)	/* don't try all 256 devices */
          break;
    }
    if (err)
	errno = err;
    return -1;
}

#ifdef HAVE_LINUX_IF_TUN_H /* New driver support */
#include <linux/if_tun.h>

#ifndef OTUNSETNOCSUM
/* pre 2.4.6 compatibility */
#define OTUNSETNOCSUM  (('T'<< 8) | 200)
#define OTUNSETDEBUG   (('T'<< 8) | 201)
#define OTUNSETIFF     (('T'<< 8) | 202)
#define OTUNSETPERSIST (('T'<< 8) | 203)
#define OTUNSETOWNER   (('T'<< 8) | 204)
#endif

static int tun_open_common(char *dev, int istun)
{
    struct ifreq ifr;
    int fd;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
       return tun_open_common0(dev, istun);

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = (istun ? IFF_TUN : IFF_TAP) | IFF_NO_PI;
    if (*dev)
       strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
       if (errno == EBADFD) {
	  /* Try old ioctl */
 	  if (ioctl(fd, OTUNSETIFF, (void *) &ifr) < 0)
	     goto failed;
       } else
          goto failed;
    }

    strcpy(dev, ifr.ifr_name);
    return fd;

failed:
    close(fd);
    return -1;
}

#else

# define tun_open_common(dev, type) tun_open_common0(dev, type)

#endif /* New driver support */

int tun_open(char *dev) { return tun_open_common(dev, 1); }
int tap_open(char *dev) { return tun_open_common(dev, 0); }

class TUN2BundleGateway : public dtn::api::Client
{
	public:
		TUN2BundleGateway(int fd, string app, ibrcommon::socketstream &stream)
		: dtn::api::Client(app, _stream), _fd(fd), _stream(stream)
		{
		};

		/**
		 * Destructor of the connection.
		 */
		virtual ~TUN2BundleGateway()
		{
			// Close the tcp connection.
			_stream.close();
		};

	private:
		// file descriptor for the tun device
		int _fd;

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

		ibrcommon::socketstream &_stream;
};

bool m_running = true;
int tunnel_fd = -1;

void term(int signal)
{
	if (signal >= 1)
	{
		m_running = false;
		::close(tunnel_fd);
		tunnel_fd = -1;
	}
}

int main(int argc, char *argv[])
{
	// catch process signals
	signal(SIGINT, term);
	signal(SIGTERM, term);

	cout << "IBR-DTN IP <-> Bundle Tunnel" << endl;

	if (argc < 5)
	{
		cout << "Syntax: " << argv[0] << " <dev> <ip> <ptp> <dst>" << endl;
		cout << "  <dev>   Virtual network device to create" << endl;
		cout << "  <ip>    Own IP address to set" << endl;
		cout << "  <ptp>   IP address of the Point-To-Point partner" << endl;
		cout << "  <dst>   EID of the destination" << endl;
		return -1;
	}

	int tunnel_fd = tun_open(argv[1]);

	if (tunnel_fd == -1)
	{
		cerr << "Error: failed to open tun device" << endl;
		return -1;
	}

	// create a connection to the dtn daemon
	ibrcommon::vaddress addr("localhost");
	ibrcommon::socketstream conn(new ibrcommon::tcpsocket(addr, 4550));
	TUN2BundleGateway gateway(tunnel_fd, "tun", conn);

	// set the interface addresses
	stringstream ifconfig;
	ifconfig << "ifconfig " << argv[1] << " -pointopoint " << argv[2] << " dstaddr " << argv[3];
	if ( system(ifconfig.str().c_str()) > 0 )
	{
		std::cerr << "can not the interface address" << std::endl;
	}

	gateway.connect();

	cout << "ready" << endl;

	while (m_running)
	{
		char data[65536];
		int ret = ::read(tunnel_fd, data, sizeof(data));

		cout << "received " << ret << " bytes" << endl;

		// create a blob
		ibrcommon::BLOB::Reference blob = ibrcommon::BLOB::create();

		// add the data
		blob.iostream()->write(data, ret);

		// create a new bundle
		dtn::api::BLOBBundle b(dtn::data::EID(argv[4]), blob);

		// transmit the packet
		gateway << b;
		gateway.flush();
	}

	gateway.close();

	::close(tunnel_fd);
	tunnel_fd = -1;

	return 0;
}
