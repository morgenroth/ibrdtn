/*
 * dtntrigger.cpp
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
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/api/Client.h>
#include <ibrcommon/net/socket.h>
#include <ibrcommon/data/File.h>

#include <csignal>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


// set this variable to false to stop the app
bool _running = true;

// global connection
ibrcommon::socketstream *_conn = NULL;

std::string _appname = "trigger";
std::string _script = "";
std::string _shell = "/bin/sh";

ibrcommon::File blob_path("/tmp");

dtn::data::EID group;

bool signed_only = false;

void print_help()
{
	cout << "-- dtntrigger (IBR-DTN) --" << endl;
	cout << "Syntax: dtntrigger [options] <name> <shell> [trigger-script]"  << endl;
	cout << "<name>        the application name" << endl;
	cout << "<shell>       shell to execute the trigger script" << endl;
	cout << "[trigger-script]  optional: the trigger script to execute on incoming bundle" << endl;
	cout << "* optional parameters *" << endl;
	cout << " -h           display this text" << endl;
	cout << " -g <group>   join a group" << endl;
	cout << " -w           temporary work directory" << endl;
	cout << " -s           process signed bundles only" << endl;
}

int init(int argc, char** argv)
{
	int index;
	int c;

	opterr = 0;

	while ((c = getopt (argc, argv, "hw:g:s")) != -1)
	switch (c)
	{
		case 'w':
			blob_path = ibrcommon::File(optarg);
			break;

		case 'g':
			group = std::string(optarg);
			break;

		case '?':
			if (optopt == 'w')
			fprintf (stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint (optopt))
			fprintf (stderr, "Unknown option `-%c'.\n", optopt);
			else
			fprintf (stderr,
					 "Unknown option character `\\x%x'.\n",
					 optopt);
			return 1;

		case 's':
			signed_only = true;
			break;

		default:
			print_help();
			return 1;
	}

	int optindex = 0;
	for (index = optind; index < argc; ++index)
	{
		switch (optindex)
		{
		case 0:
			_appname = std::string(argv[index]);
			break;

		case 1:
			_shell = std::string(argv[index]);
			break;

		case 2:
			_script = std::string(argv[index]);
			break;
		}

		optindex++;
	}

	// print help if not enough parameters are set
	if (optindex < 2) { print_help(); exit(0); }

	// enable file based BLOBs if a correct path is set
	if (blob_path.exists())
	{
		ibrcommon::BLOB::changeProvider(new ibrcommon::FileBLOBProvider(blob_path));
	}

	return 0;
}

void term(int signal)
{
	if (signal >= 1)
	{
		_running = false;
		if (_conn != NULL) _conn->close();
	}
}

/*
 * main application method
 */
int main(int argc, char** argv)
{
	// catch process signals
	signal(SIGINT, term);
	signal(SIGTERM, term);

	// read the configuration
	if (init(argc, argv) > 0)
	{
		return (EXIT_FAILURE);
	}

	// backoff for reconnect
	size_t backoff = 2;

	// loop, if no stop if requested
	while (_running)
	{
		try {
			// Create a stream to the server using TCP.
			ibrcommon::vaddress addr("localhost", 4550);
			ibrcommon::socketstream conn(new ibrcommon::tcpsocket(addr));

			// set the connection globally
			_conn = &conn;

			// Initiate a client for synchronous receiving
			dtn::api::Client client(_appname, group, conn);

			// Connect to the server. Actually, this function initiate the
			// stream protocol by starting the thread and sending the contact header.
			client.connect();

			// reset backoff if connected
			backoff = 2;

			// check the connection
			while (_running)
			{
				// receive the bundle
				dtn::data::Bundle b = client.getBundle();

				// skip non-signed bundles if we should accept signed bundles only
				if (signed_only && !b.get(dtn::data::PrimaryBlock::DTNSEC_STATUS_VERIFIED)) continue;

				// get the reference to the blob
				ibrcommon::BLOB::Reference ref = b.find<dtn::data::PayloadBlock>().getBLOB();

				// get a temporary file name
				ibrcommon::TemporaryFile file(blob_path, "bundle");

				// write data to temporary file
				try {
					std::fstream out(file.getPath().c_str(), ios::out|ios::binary|ios::trunc);
					out.exceptions(std::ios::badbit | std::ios::eofbit);
					out << ref.iostream()->rdbuf();
					out.close();

					// call the script
					std::string cmd = _shell + " " + _script + " " + b._source.getString() + " " + file.getPath();
					::system(cmd.c_str());

					// remove temporary file
					file.remove();
				} catch (const ios_base::failure&) {

				}
			}

			// close the client connection
			client.close();

			// close the connection
			conn.close();

			// set the global connection to NULL
			_conn = NULL;
		} catch (const ibrcommon::socket_exception&) {
			// set the global connection to NULL
			_conn = NULL;

			if (_running)
			{
				cout << "Connection to bundle daemon failed. Retry in " << backoff << " seconds." << endl;
				sleep(backoff);

				// if backoff < 10 minutes
				if (backoff < 600)
				{
					// set a new backoff
					backoff = backoff * 2;
				}
			}
		} catch (const ibrcommon::IOException &ex) {
			// set the global connection to NULL
			_conn = NULL;

			if (_running)
			{
				cout << "Connection to bundle daemon failed. Retry in " << backoff << " seconds." << endl;
				sleep(backoff);

				// if backoff < 10 minutes
				if (backoff < 600)
				{
					// set a new backoff
					backoff = backoff * 2;
				}
			}
		} catch (const std::exception&) {
			// set the global connection to NULL
			_conn = NULL;
		}
	}

	return (EXIT_SUCCESS);
}
