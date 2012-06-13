/*
 * dtnrecv-ng.cpp
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
#include "ibrdtn/api/APIClient.h"
#include "ibrdtn/api/FileBundle.h"
#include "ibrcommon/net/tcpclient.h"

#include <csignal>
#include <sys/types.h>
#include <iostream>

void print_help()
{
	cout << "-- dtnrecv-ng (IBR-DTN) --" << endl;
	cout << "Syntax: dtnrecv-ng [options]"  << endl;
	cout << "* optional parameters *" << endl;
	cout << " -h|--help            display this text" << endl;
	cout << " --file <filename>    write the incoming data to the a file instead of the standard output" << endl;
	cout << " --name <name>        set the application name (e.g. filetransfer)" << endl;
//	cout << " --timeout <seconds>  receive timeout in seconds" << endl;
	cout << " --count <number>     receive that many bundles" << endl;
	cout << " -U <socket>     use UNIX domain sockets" << endl;
}

dtn::api::APIClient *_client = NULL;
ibrcommon::tcpclient *_conn = NULL;

int h = 0;
bool _stdout = true;

void term(int signal)
{
	if (!_stdout)
	{
		std::cout << h << " bundles received." << std::endl;
	}

	if (signal >= 1)
	{
		if (_client != NULL)
		{
			_client->unblock_wait();
			_conn->close();
		}
	}
}

int main(int argc, char *argv[])
{
	// catch process signals
	signal(SIGINT, term);
	signal(SIGTERM, term);

	int ret = EXIT_SUCCESS;
	string filename = "";
	string name = "filetransfer";
	int timeout = 0;
	int count   = 1;
	ibrcommon::File unixdomain;

	for (int i = 0; i < argc; i++)
	{
		string arg = argv[i];

		// print help if requested
		if (arg == "-h" || arg == "--help")
		{
			print_help();
			return ret;
		}

		if (arg == "--name" && argc > i)
		{
			name = argv[i + 1];
		}

		if (arg == "--file" && argc > i)
		{
			filename = argv[i + 1];
			_stdout = false;
		}

		if (arg == "--timeout" && argc > i)
		{
			timeout = atoi(argv[i + 1]);
		}

		if (arg == "--count" && argc > i)
		{
			count = atoi(argv[i + 1]);
		}

		if (arg == "-U" && argc > i)
		{
			if (++i > argc)
			{
				std::cout << "argument missing!" << std::endl;
				return -1;
			}

			unixdomain = ibrcommon::File(argv[i]);
		}
	}

	try {
		// Create a stream to the server using TCP.
		ibrcommon::tcpclient conn;

		// check if the unixdomain socket exists
		if (unixdomain.exists())
		{
			// connect to the unix domain socket
			conn.open(unixdomain);
		}
		else
		{
			// connect to the standard local api port
			conn.open("127.0.0.1", 4550);

			// enable nodelay option
			conn.enableNoDelay();
		}

		// Initiate a client for synchronous receiving
		dtn::api::APIClient client(conn);

		// read the connection banner
		client.connect();

		// export objects for the signal handler
		_conn = &conn;
		_client = &client;

		// register endpoint identifier
		client.setEndpoint(name);

		std::fstream file;

		if (!_stdout)
		{
			std::cout << "Wait for incoming bundle... " << std::endl;
			file.open(filename.c_str(), ios::in|ios::out|ios::binary|ios::trunc);
			file.exceptions(std::ios::badbit | std::ios::eofbit);
		}

		h = 0;
		while ((h < count) && !conn.eof())
		{
			// receive the next notify
			dtn::api::APIClient::Message n = client.wait();

			try {
				if (n.code == dtn::api::APIClient::API_STATUS_NOTIFY_BUNDLE)
				{
					dtn::api::Bundle b = client.get();

					// get the reference to the blob
					ibrcommon::BLOB::Reference ref = b.getData();

					// write the data to output
					if (_stdout)
					{
						cout << ref.iostream()->rdbuf();
					}
					else
					{
						// write data to temporary file
						try {
							std::cout << "Bundle received (" << (h + 1) << ")." << endl;

							file << ref.iostream()->rdbuf();
						} catch (const ios_base::failure&) {

						}
					}

					// bundle received, increment counter
					h++;

					// notify it as delivered
					client.notify_delivered(b);
				}
			} catch (const ibrcommon::Exception&) {
				// bundle get failed
				std::cerr << "bundle get failed" << std::endl;
			}
		}

		if (!_stdout)
		{
			file.close();
			std::cout << "done." << std::endl;
		}

		// close the tcp connection
		conn.close();
	} catch (const std::exception &ex) {
		std::cerr << "Error: " << ex.what() << std::endl;
		ret = EXIT_FAILURE;
	}

	return ret;
}

