/*
 * dtnping.cpp
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

#include "config.h"
#include "ibrdtn/api/Client.h"
#include "ibrdtn/api/StringBundle.h"
#include "ibrcommon/net/socket.h"
#include "ibrcommon/thread/Mutex.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/TimeMeasurement.h"

#include <iostream>
#include <csignal>
#include <stdint.h>

#define CREATE_CHUNK_SIZE 2048

class EchoClient : public dtn::api::Client
{
	public:
		EchoClient(dtn::api::Client::COMMUNICATION_MODE mode, string app, ibrcommon::socketstream &stream)
		 : dtn::api::Client(app, _stream, mode), _stream(stream)
		{
			seq=0;
		}

		virtual ~EchoClient()
		{
		}

		const dtn::api::Bundle waitForReply(const int timeout)
		{
			double wait=(timeout*1000);
			ibrcommon::TimeMeasurement tm;
			while ( wait > 0 )
			{
				try {
					tm.start();
					dtn::api::Bundle b = this->getBundle((int)(wait/1000));
					tm.stop();
					checkReply(b);
					return b;
				} catch (const std::string &errmsg) {
					std::cerr << errmsg << std::endl;
				}
				wait=wait-tm.getMilliseconds();
			}
			throw ibrcommon::Exception("timeout is set to zero");
		}

		void echo(EID destination, int size, int lifetime, bool encryption = false, bool sign = false)
		{
			lastdestination=destination.getString();
			seq++;
			
			// create a bundle
			dtn::api::StringBundle b(destination);

			// enable encryption if requested
			if (encryption) b.requestEncryption();

			// enable signature if requested
			if (sign) b.requestSigned();

			// set lifetime
			b.setLifetime(lifetime);
			
			//Add magic seqnr. Hmm, how to to do this without string?
			b.append(string((char *)(&seq),4));
			
			if (size > 4)
			{
				size-=4;

				// create testing pattern, chunkwise to ocnserve memory
				char pattern[CREATE_CHUNK_SIZE];
				for (size_t i = 0; i < sizeof(pattern); i++)
				{
					pattern[i] = '0';
					pattern[i] += i % 10;
				}
				string chunk=string(pattern,CREATE_CHUNK_SIZE);

				while (size > CREATE_CHUNK_SIZE) {
					b.append(chunk);
					size-=CREATE_CHUNK_SIZE;
				}
				b.append(chunk.substr(0,size));
			}

			// send the bundle
			(*this) << b;

			// ... flush out
			flush();
			
		}
		
		void checkReply(dtn::api::Bundle &bundle) {
			size_t reply_seq = 0;
			ibrcommon::BLOB::Reference blob = bundle.getData();
			blob.iostream()->read((char *)(&reply_seq),4 );

			if (reply_seq != seq) {
				std::stringstream ss;
				ss << "sequence number mismatch, awaited " << seq << ", got " << reply_seq;
				throw ss.str();
			}
			if (bundle.getSource().getString() != lastdestination) {
				throw std::string("ignoring bundle from source " + bundle.getSource().getString() + " awaited " + lastdestination);
			}
		}

	private:
		ibrcommon::socketstream &_stream;
		uint32_t seq;
		string lastdestination;
};


	
void print_help()
{
	cout << "-- dtnping (IBR-DTN) --" << endl;
	cout << "Syntax: dtnping [options] <dst>"  << endl;
	cout << " <dst>    set the destination eid (e.g. dtn://node/echo)" << endl;
	cout << "* optional parameters *" << endl;
	cout << " -h|--help       display this text" << endl;
	cout << " --src <name>    set the source application name (e.g. echo-client)" << endl;
	cout << " --nowait        do not wait for a reply" << endl;
	cout << " --abortfail     Abort after first packetloss" << endl;
	cout << " --size          the size of the payload" << endl;
	cout << " --count X       send X echo in a row" << endl;
	cout << " --delay X       delay (seconds) after a successful response" << endl;
	cout << " --lifetime <seconds> set the lifetime of outgoing bundles; default: 30" << endl;
	cout << " --encrypt       request encryption on the bundle layer" << endl;
	cout << " --sign          request signature on the bundle layer" << endl;
	cout << " -U <socket>     use UNIX domain sockets" << endl;
}

size_t _received = 0, _transmitted = 0;
double _min = 0.0, _max = 0.0, _avg = 0.0;
ibrcommon::TimeMeasurement _runtime;
ibrcommon::Conditional __pause;
dtn::api::Client *__client = NULL;

EID _addr;
bool __exit = false;

void print_summary()
{
	_runtime.stop();

	double loss = 0; if (_transmitted > 0) loss = (((double)_transmitted - (double)_received) / (double)_transmitted) * 100.0;
	double avg_value = 0; if (_received > 0) avg_value = (_avg/_received);

	std::cout << std::endl << "--- " << _addr.getString() << " echo statistics --- " << std::endl;
	std::cout << _transmitted << " bundles transmitted, " << _received << " received, " << loss << "% bundle loss, time " << _runtime << std::endl;
	std::cout << "rtt min/avg/max = ";
	ibrcommon::TimeMeasurement::format(std::cout, _min) << "/";
	ibrcommon::TimeMeasurement::format(std::cout, avg_value) << "/";
	ibrcommon::TimeMeasurement::format(std::cout, _max) << " ms" << std::endl;
}

void term(int signal)
{
	if (signal >= 1)
	{
		if (!__exit)
		{
			ibrcommon::MutexLock l(__pause);
			if (__client != NULL) __client->abort();
			__exit = true;
			__pause.abort();
		}
	}
}

int main(int argc, char *argv[])
{
	// catch process signals
	signal(SIGINT, term);
	signal(SIGTERM, term);

	string ping_destination = "dtn://local/echo";
	string ping_source = "";
	int ping_size = 64;
	unsigned int lifetime = 30;
	bool wait_for_reply = true;
	bool stop_after_first_fail = false;
	bool nonstop = true;
	size_t interval_pause = 1;
	size_t count = 0;
	dtn::api::Client::COMMUNICATION_MODE mode = dtn::api::Client::MODE_BIDIRECTIONAL;
	ibrcommon::File unixdomain;
	bool bundle_encryption = false;
	bool bundle_signed = false;

	if (argc == 1)
	{
		print_help();
		return 0;
	}

	for (int i = 1; i < argc; i++)
	{
		string arg = argv[i];

		// print help if requested
		if ((arg == "-h") || (arg == "--help"))
		{
			print_help();
			return 0;
		}

		else if (arg == "--encrypt")
		{
			bundle_encryption = true;
		}

		else if (arg == "--sign")
		{
			bundle_signed = true;
		}

		else if (arg == "--nowait")
		{
			mode = dtn::api::Client::MODE_SENDONLY;
			wait_for_reply = false;
		}
		
		else if ( arg == "--abortfail") {
			stop_after_first_fail=true;
		}

		else if (arg == "--src" && argc > i)
		{
			ping_source = argv[i + 1];
			i++;
		}

		else if (arg == "--size" && argc > i)
		{
			stringstream str_size;
			str_size.str( argv[i + 1] );
			str_size >> ping_size;
			i++;
		}

		else if (arg == "--count" && argc > i)
		{
			stringstream str_count;
			str_count.str( argv[i + 1] );
			str_count >> count;
			i++;
			nonstop = false;
		}

		else if (arg == "--delay" && argc > i)
		{
			stringstream str_delay;
			str_delay.str( argv[i + 1] );
			str_delay >> interval_pause;
			i++;
		}

		else if (arg == "--lifetime" && argc > i)
		{
			stringstream data; data << argv[i + 1];
			data >> lifetime;
			i++;
		}
		else if (arg == "-U" && argc > i)
		{
			if (++i > argc)
			{
					std::cout << "argument missing!" << std::endl;
					return -1;
			}

			unixdomain = ibrcommon::File(argv[i]);
		}
	}

	// the last parameter is always the destination
	ping_destination = argv[argc - 1];

	// target address
	_addr = EID(ping_destination);

	ibrcommon::TimeMeasurement tm;
	
	
	try {
		// Create a stream to the server using TCP.
		ibrcommon::clientsocket *sock = NULL;

		// check if the unixdomain socket exists
		if (unixdomain.exists())
		{
			// connect to the unix domain socket
			sock = new ibrcommon::filesocket(unixdomain);
		}
		else
		{
			// connect to the standard local api port
			ibrcommon::vaddress addr("localhost", 4550);
			sock = new ibrcommon::tcpsocket(addr);
		}

		ibrcommon::socketstream conn(sock);

		// Initiate a derivated client
		EchoClient client(mode, ping_source, conn);

		// set the global client pointer
		{
			ibrcommon::MutexLock l(__pause);
			__client = &client;
		}

		// Connect to the server. Actually, this function initiate the
		// stream protocol by starting the thread and sending the contact header.
		client.connect();

		std::cout << "ECHO " << _addr.getString() << " " << ping_size << " bytes of data." << std::endl;

		// measure runtime
		_runtime.start();

		try {
			for (unsigned int i = 0; (i < count) || nonstop; i++)
			{
				// set sending time
				tm.start();

				// Call out a ECHO
				client.echo( _addr, ping_size, lifetime, bundle_encryption, bundle_signed );
				_transmitted++;
			
				if (wait_for_reply)
				{
					try {
						try {
							dtn::api::Bundle response = client.waitForReply(2*lifetime);

							// print out measurement result
							tm.stop();

							size_t reply_seq = 0;
							size_t payload_size = 0;

							// check for min/max/avg
							_avg += tm.getMilliseconds();
							if ((_min > tm.getMilliseconds()) || _min == 0) _min = tm.getMilliseconds();
							if ((_max < tm.getMilliseconds()) || _max == 0) _max = tm.getMilliseconds();

							{
								ibrcommon::BLOB::Reference blob = response.getData();
								blob.iostream()->read((char *)(&reply_seq),4 );
								payload_size = blob.iostream().size();
							}

							std::cout << payload_size << " bytes from " << response.getSource().getString() << ": seq=" << reply_seq << " ttl=" << response.getLifetime() << " time=" << tm << std::endl;
							_received++;
						} catch (const dtn::api::ConnectionTimeoutException &e) {
							if (stop_after_first_fail)
								throw dtn::api::ConnectionTimeoutException();

							std::cout << "Timeout." << std::endl;
						}

						if (interval_pause > 0)
						{
							ibrcommon::MutexLock l(__pause);
							__pause.wait(interval_pause * 1000);
						}
					} catch (const ibrcommon::Conditional::ConditionalAbortException &e) {
						if (e.reason == ibrcommon::Conditional::ConditionalAbortException::COND_TIMEOUT)
						{
							continue;
						}
						// aborted
						break;
					} catch (const dtn::api::ConnectionAbortedException&) {
						// aborted... do a clean shutdown
						break;
					} catch (const dtn::api::ConnectionTimeoutException &ex) {
						if (stop_after_first_fail)
						{
							std::cout << "No response, aborting." << std::endl;
							break;
						}
					}
				}
			}
		} catch (const dtn::api::ConnectionException&) {
			std::cerr << "Disconnected." << std::endl;
		} catch (const ibrcommon::IOException&) {
			std::cerr << "Error while receiving a bundle." << std::endl;
		}

		// Shutdown the client connection.
		client.close();
		conn.close();
	} catch (const ibrcommon::socket_exception&) {
		std::cerr << "Can not connect to the daemon. Does it run?" << std::endl;
		return -1;
	} catch (const std::exception&) {
		std::cerr << "unknown error" << std::endl;
	}

	print_summary();

	return 0;
}
