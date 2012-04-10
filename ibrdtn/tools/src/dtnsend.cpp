/*
 * dtnsend.cpp
 *
 *  Created on: 15.10.2009
 *      Author: morgenro
 */

#include "config.h"
#include <ibrdtn/api/Client.h>
#include <ibrdtn/api/FileBundle.h>
#include <ibrdtn/api/BLOBBundle.h>
#include <ibrcommon/net/tcpclient.h>
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/Logger.h>

#include <iostream>

void print_help()
{
	cout << "-- dtnsend (IBR-DTN) --" << endl;
	cout << "Syntax: dtnsend [options] <dst> <filename>"  << endl;
	cout << " <dst>         set the destination eid (e.g. dtn://node/filetransfer)" << endl;
	cout << " <filename>    the file to transfer" << endl;
	cout << "* optional parameters *" << endl;
	cout << " -h|--help     display this text" << endl;
	cout << " --src <name>  set the source application name (e.g. filetransfer)" << endl;
	cout << " -p <0..2>     set the bundle priority (0 = low, 1 = normal, 2 = high)" << endl;
	cout << " -g            receiver is a destination group" << endl;
	cout << " --lifetime <seconds>" << endl;
	cout << "               set the lifetime of outgoing bundles; default: 3600" << endl;
	cout << " -U <socket>   use UNIX domain sockets" << endl;
	cout << " -n <copies>   create <copies> bundle copies" << endl;
	cout << " --encrypt     request encryption on the bundle layer" << endl;
	cout << " --sign        request signature on the bundle layer" << endl;
	cout << " --custody     request custody transfer of the bundle" << endl;
	cout << " --compression request compression of the payload" << endl;

}

int main(int argc, char *argv[])
{
	bool error = false;
	string file_destination = "dtn://local/filetransfer";
	string file_source = "";
	unsigned int lifetime = 3600;
	bool use_stdin = false;
	std::string filename;
	ibrcommon::File unixdomain;
	int priority = 1;
	int copies = 1;
	bool bundle_encryption = false;
	bool bundle_signed = false;
	bool bundle_custody = false;
	bool bundle_compression = false;
	bool bundle_group = false;

//	ibrcommon::Logger::setVerbosity(99);
//	ibrcommon::Logger::addStream(std::cout, ibrcommon::Logger::LOGGER_ALL, ibrcommon::Logger::LOG_DATETIME | ibrcommon::Logger::LOG_LEVEL);

	std::list<std::string> arglist;

	for (int i = 0; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			std::string arg = argv[i];

			// print help if requested
			if (arg == "-h" || arg == "--help")
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
			else if (arg == "--custody")
			{
				bundle_custody = true;
			}
			else if (arg == "--compression")
			{
				bundle_compression = true;
			}
			else if (arg == "--src" && argc > i)
			{
				if (++i > argc)
				{
					std::cout << "argument missing!" << std::endl;
					return -1;
				}

				file_source = argv[i];
			}
			else if (arg == "--lifetime" && argc > i)
			{
				if (++i > argc)
				{
					std::cout << "argument missing!" << std::endl;
					return -1;
				}

				stringstream data; data << argv[i];
				data >> lifetime;
			}
			else if (arg == "-p" && argc > i)
			{
				if (++i > argc)
				{
					std::cout << "argument missing!" << std::endl;
					return -1;
				}
				stringstream data; data << argv[i];
				data >> priority;
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
			else if (arg == "-n" && argc > i)
			{
				if (++i > argc)
				{
					std::cout << "argument missing!" << std::endl;
					return -1;
				}

				stringstream data; data << argv[i];
				data >> copies;

				if( copies < 1 ) {
					std::cout << "invalid number of bundle copies!" << std::endl;
					return -1;
				}
			}
			else if (arg == "-g")
			{
				bundle_group = true;
			}
			else
			{
				std::cout << "invalid argument " << arg << std::endl;
				return -1;
			}
		}
		else
		{
			arglist.push_back(argv[i]);
		}
	}

	if (arglist.size() <= 1)
	{
		print_help();
		return -1;
	} else if (arglist.size() == 2)
	{
		std::list<std::string>::iterator iter = arglist.begin(); iter++;

		// the first parameter is the destination
		file_destination = (*iter);

		use_stdin = true;
	}
	else if (arglist.size() > 2)
	{
		std::list<std::string>::iterator iter = arglist.begin(); iter++;

		// the first parameter is the destination
		file_destination = (*iter); iter++;

		// the second parameter is the filename
		filename = (*iter);
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

		try {
			// Initiate a client for synchronous receiving
			dtn::api::Client client(file_source, conn, dtn::api::Client::MODE_SENDONLY);

			// Connect to the server. Actually, this function initiate the
			// stream protocol by starting the thread and sending the contact header.
			client.connect();

			// target address
			EID addr = EID(file_destination);

			try {
				if (use_stdin)
				{
					cout << "Transfer stdin to " << addr.getString() << endl;

					// create an empty BLOB
					ibrcommon::BLOB::Reference ref = ibrcommon::BLOB::create();

					// copy cin to a BLOB
					(*ref.iostream()) << cin.rdbuf();

					for(int u=0; u<copies; u++){
						dtn::api::BLOBBundle b(file_destination, ref);

						// set destination address to non-singleton
						if (bundle_group) b.setSingleton(false);

						// enable encryption if requested
						if (bundle_encryption) b.requestEncryption();

						// enable signature if requested
						if (bundle_signed) b.requestSigned();

						// enable custody transfer if requested
						if (bundle_custody) b.requestCustodyTransfer();

						// enable compression
						if (bundle_compression) b.requestCompression();

						// set the lifetime
						b.setLifetime(lifetime);

						// set the bundles priority
						b.setPriority(dtn::api::Bundle::BUNDLE_PRIORITY(priority));

						// send the bundle
						client << b;

						if (copies > 1)
						{
							std::cout << "sent copy #" << (u+1) << std::endl;
						}
					}
				}
				else
				{
					cout << "Transfer file \"" << filename << "\" to " << addr.getString() << endl;
					
					for(int u=0; u<copies; u++){
						// create a bundle from the file
						dtn::api::FileBundle b(file_destination, filename);

						// set destination address to non-singleton
						if (bundle_group) b.setSingleton(false);

						// enable encryption if requested
						if (bundle_encryption) b.requestEncryption();

						// enable signature if requested
						if (bundle_signed) b.requestSigned();

						// enable custody transfer if requested
						if (bundle_custody) b.requestCustodyTransfer();

						// enable compression
						if (bundle_compression) b.requestCompression();

						// set the lifetime
						b.setLifetime(lifetime);

						// set the bundles priority
						b.setPriority(dtn::api::Bundle::BUNDLE_PRIORITY(priority));

						// send the bundle
						client << b;

						if (copies > 1)
						{
							std::cout << "sent copy #" << (u+1) << std::endl;
						}
					}
				}

				// flush the buffers
				client.flush();
			} catch (const ibrcommon::IOException &ex) {
				std::cerr << "Error while sending bundle." << std::endl;
				std::cerr << "\t" << ex.what() << std::endl;
				error = true;
			}

			// Shutdown the client connection.
			client.close();

		} catch (const ibrcommon::IOException &ex) {
			cout << "Error: " << ex.what() << endl;
			error = true;
		} catch (const dtn::api::ConnectionException&) {
			// connection already closed, the daemon was faster
		}

		// close the tcpstream
		conn.close();
	} catch (const std::exception &ex) {
		cout << "Error: " << ex.what() << endl;
		error = true;
	}

	if (error) return -1;

	return 0;
}
