/*
 * dtninbox.cpp
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
#include "ibrdtn/api/FileBundle.h"
#include "ibrcommon/net/tcpclient.h"
#include "ibrcommon/thread/Mutex.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrdtn/data/PayloadBlock.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrcommon/data/BLOB.h"
#include "ibrcommon/data/File.h"
#include "ibrcommon/appstreambuf.h"

#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <csignal>
#include <sys/types.h>
#include <unistd.h>

using namespace ibrcommon;

void print_help()
{
        cout << "-- dtninbox (IBR-DTN) --" << endl;
        cout << "Syntax: dtninbox [options] <name> <inbox>"  << endl;
        cout << " <name>           the application name" << endl;
        cout << " <inbox>          directory where incoming files should be placed" << endl;
        cout << "* optional parameters *" << endl;
        cout << " -h|--help        display this text" << endl;
        cout << " -w|--workdir     temporary work directory" << endl;
}

map<string,string> readconfiguration(int argc, char** argv)
{
    // print help if not enough parameters are set
    if (argc < 3) { print_help(); exit(0); }

    map<string,string> ret;

    ret["name"] = argv[argc - 2];
    ret["inbox"] = argv[argc - 1];

    for (int i = 0; i < (argc - 2); i++)
    {
        string arg = argv[i];

        // print help if requested
        if (arg == "-h" || arg == "--help")
        {
            print_help();
            exit(0);
        }

        if ((arg == "-w" || arg == "--workdir") && (argc > i))
        {
            ret["workdir"] = argv[i + 1];
        }
    }

    return ret;
}

// set this variable to false to stop the app
bool _running = true;

// global connection
ibrcommon::tcpclient *_conn = NULL;

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
    map<string,string> conf = readconfiguration(argc, argv);

    // init working directory
    if (conf.find("workdir") != conf.end())
    {
    	ibrcommon::File blob_path(conf["workdir"]);

    	if (blob_path.exists())
    	{
    		ibrcommon::BLOB::changeProvider(new ibrcommon::FileBLOBProvider(blob_path), true);
    	}
    }

    // backoff for reconnect
    size_t backoff = 2;

    // check outbox for files
	File outbox(conf["outbox"]);

    // loop, if no stop if requested
    while (_running)
    {
        try {
        	// Create a stream to the server using TCP.
        	ibrcommon::tcpclient conn("127.0.0.1", 4550);

    		// enable nodelay option
    		conn.enableNoDelay();

        	// set the connection globally
        	_conn = &conn;

            // Initiate a client for synchronous receiving
            dtn::api::Client client(conf["name"], conn);

            // Connect to the server. Actually, this function initiate the
            // stream protocol by starting the thread and sending the contact header.
            client.connect();

            // reset backoff if connected
            backoff = 2;

            // check the connection
            while (_running)
            {
            	// receive the bundle
            	dtn::api::Bundle b = client.getBundle();

            	// get the reference to the blob
            	ibrcommon::BLOB::Reference ref = b.getData();

                // create the extract command
                stringstream cmdstream; cmdstream << "tar -x -C " << conf["inbox"];

                // create a tar handler
                appstreambuf extractor(cmdstream.str(), appstreambuf::MODE_WRITE);
                ostream stream(&extractor);

                // write the payload to the extractor
               	stream << ref.iostream()->rdbuf();

                // flush the stream
                stream.flush();
            }

            // close the client connection
            client.close();

            // close the connection
            conn.close();

            // set the global connection to NULL
            _conn = NULL;
        } catch (const ibrcommon::tcpclient::SocketException&) {
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
        } catch (const ibrcommon::IOException&) {
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
