/*
 * dtnoutbox.cpp
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
#include "ibrcommon/net/socket.h"
#include "ibrcommon/thread/Mutex.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrdtn/data/PayloadBlock.h"
#include "ibrcommon/data/BLOB.h"
#include "ibrcommon/data/File.h"
#include "ibrcommon/appstreambuf.h"

#include "ToolUtils.h"
extern "C"
{
#include "tffs/tffs.h"
}

#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
#ifdef HAVE_LIBARCHIVE
#include <archive.h>
#include <archive_entry.h>
#include <fcntl.h>
#endif

using namespace ibrcommon;

void print_help()
{
	cout << "-- dtnoutbox (IBR-DTN) --" << endl;
	cout << "Syntax: dtnoutbox [options] <name> <outbox> <destination>" << endl;
	cout << " <name>           the application name" << endl;
	cout << " <outbox>         directory with outgoing files" << endl;
	cout << " <destination>    the destination EID for all outgoing files" << endl;
	cout << "* optional parameters *" << endl;
	cout << " -h|--help        display this text" << endl;
	cout << " -w|--workdir     temporary work directory" << endl;
	cout << " -k|--keep        keep files in outbox" << endl;
	cout << " -i <interval>	   interval in milliseconds, in which <outbox> is scanned for new/changed files. default: 5000" << endl;
	cout << " -r <number>	   number of rounds of intervals, after which a unchanged file is considered as written. default: 3" << endl;

}

map<string,string> readconfiguration( int argc, char** argv )
{
	// print help if not enough parameters are set
	if (argc < 4)
	{
		print_help();
		exit(0);
	}

	map<string,string> ret;
	ret["name"] = argv[1];
	ret["outbox"] = argv[2];
	ret["destination"] = argv[3];
	ret["interval"] = "5000"; //default value
	ret["rounds"] = "3"; //default value

	for (int i = 4; i < argc; ++i)
	{
		string arg = argv[i];

		// print help if requested
		if (arg == "-h" || arg == "--help")
		{
			print_help();
			exit(0);
		}

		if (arg == "-w" || arg == "--workdir")
		{
			ret["workdir"] = argv[++i];
		}

		if (arg == "-k" || arg == "--keep")
		{
			ret["keep"] = "1";
		}

		if (arg == "-i")
		{
			ret["interval"] = argv[++i];
		}

		if (arg == "-r")
		{
			ret["rounds"] = argv[++i];
		}

	}

	return ret;
}

// set this variable to false to stop the app
bool _running = true;

// global connection
ibrcommon::socketstream *_conn = NULL;

void term( int signal )
{
	if (signal >= 1)
	{
		_running = false;
		if (_conn != NULL) _conn->close();
	}
}



class ObservedFile
{
public:
	ObservedFile( File file );
	~ObservedFile();
	bool lastSizesEqual( size_t n );
	void addSize();
	void send();
	size_t getLastSent();
	File file;
	size_t last_sent;
	vector<size_t> sizes;

};

ObservedFile::ObservedFile( File file )
		: file(file), last_sent(0)
{
}
ObservedFile::~ObservedFile()
{
	sizes.clear();
}
bool ObservedFile::lastSizesEqual( size_t n )
{
	if (n > sizes.size()) return false;

	for (size_t i = 1; i <= n; i++)
	{
		if (sizes.at(sizes.size() - i) != sizes.at(sizes.size() - i - 1)) return false;
	}
	return true;
}
void ObservedFile::addSize()
{
	sizes.push_back(file.size());
}
void ObservedFile::send()
{
	time_t timev;
	last_sent = time(&timev);
	sizes.clear();
}

/*
 * main application method
 */
int main( int argc, char** argv )
{
	bool keep_files = false;
	// catch process signals
	signal(SIGINT, term);
	signal(SIGTERM, term);

	// read the configuration
	map<string,string> conf = readconfiguration(argc, argv);
	size_t _conf_interval = atoi(conf["interval"].c_str());
	size_t _conf_rounds = atoi(conf["rounds"].c_str());

	//init list of observered files
	list<ObservedFile> observed_files;

	// init working directory
	if (conf.find("workdir") != conf.end())
	{
		ibrcommon::File blob_path(conf["workdir"]);

		if (blob_path.exists())
		{
			ibrcommon::BLOB::changeProvider(new ibrcommon::FileBLOBProvider(blob_path), true);
		}
	}

	//check keep parameter
	if (conf.find("keep") != conf.end())
	{
		keep_files = true;
	}

	// backoff for reconnect
	unsigned int backoff = 2;

	// check outbox for files
	File outbox(conf["outbox"]);

	// loop, if no stop if requested
	while (_running)
	{
		try
		{
			// Create a stream to the server using TCP.
			ibrcommon::vaddress addr("localhost", 4550);
			ibrcommon::socketstream conn(new ibrcommon::tcpsocket(addr));

			// set the connection globally
			_conn = &conn;

			// Initiate a client for synchronous receiving
			dtn::api::Client client(conf["name"], conn, dtn::api::Client::MODE_SENDONLY);

			// Connect to the server. Actually, this function initiate the
			// stream protocol by starting the thread and sending the contact header.
			client.connect();

			// reset backoff if connected
			backoff = 2;

			// check the connection
			while (_running)
			{
				list<File> files;
				outbox.getFiles(files);
				for (list<File>::iterator iter = files.begin(); iter != files.end(); ++iter)
				{
					// skip system files ("." and "..")
					if ((*iter).isSystem()) continue;

					//only add file if its not under observation
					bool new_file = true;
					for (list<ObservedFile>::iterator of_iter = observed_files.begin(); of_iter != observed_files.end(); ++of_iter)
					{
						if ((*iter).getPath() == (*of_iter).file.getPath())
						{
							new_file = false;
							break;
						}

					}
					if (new_file) observed_files.push_back(ObservedFile(*iter));
				}

				if (observed_files.size() == 0)
				{
					cout << "no files to send " << endl;
					// wait some seconds
					ibrcommon::Thread::sleep(_conf_interval);

					continue;
				}

				stringstream files_to_send;
				const char **files_to_send_ptr = new const char*[observed_files.size()];
				size_t counter = 0;
				for (list<ObservedFile>::iterator iter = observed_files.begin(); iter != observed_files.end(); ++iter)
				{
					ObservedFile &of = (*iter);

					//check existance
					if (!of.file.exists())
					{
						observed_files.erase(iter);
						//if no files to observe: stop, otherwise continue with next file
						if (observed_files.size() == 0)
							break;
						else
							continue;
					}

					// add the file to the files_to_send, if it has not been sent (with the latest timestamp)...
					size_t latest_timestamp = max(of.file.lastmodify(), of.file.laststatchange());
					of.addSize();

					if (of.last_sent < latest_timestamp)
					{
						//... and its size did not change the last x rounds
						if (of.lastSizesEqual(_conf_rounds))
						{
							of.send();
							files_to_send << of.file.getBasename() << " ";
							files_to_send_ptr[counter++] = of.file.getPath().c_str();
						}
					}
				}

				if (!counter)
				{
					cout << "no files to send " << endl;
				}
				else
				{
					cout << "files: " << files_to_send.str() << endl;

					// create a blob
					ibrcommon::BLOB::Reference blob = ibrcommon::BLOB::create();

//use libarchive or commandline fallback
#ifdef HAVE_LIBARCHIVE
					ToolUtils::write_tar_archive(&blob, files_to_send_ptr, counter);

					//delete files, if wanted
					if (!keep_files)
					{
						list<File>::iterator file_iter = files.begin();
						while (file_iter != files.end())
						{
							(*file_iter++).remove(false);
						}
					}
#else
					// "--remove-files" deletes files after adding
					//depending on configuration, this option is passed to tar or not
					std::string remove_string = " --remove-files";
					if(keep_files)
						remove_string = "";
					stringstream cmd;
					cmd << "tar" << remove_string << " -cO -C " << outbox.getPath() << " " << file_list.str();

					// make a tar archive
					appstreambuf app(cmd.str(), appstreambuf::MODE_READ);
					istream stream(&app);

					// stream the content of "tar" to the payload block
					(*blob.iostream()) << stream.rdbuf();
#endif
					// create a new bundle
					dtn::data::EID destination = EID(conf["destination"]);

					// create a new bundle
					dtn::data::Bundle b;

					// set destination
					b.destination = destination;

					// add payload block using the blob
					b.push_back(blob);

					// send the bundle
					client << b;
					client.flush();
				}
				if (_running)
				{
					// wait some seconds
					ibrcommon::Thread::sleep(_conf_interval);
				}
			}

			// close the client connection
			client.close();

			// close the connection
			conn.close();

			// set the global connection to NULL
			_conn = NULL;
		} catch (const ibrcommon::socket_exception&)
		{
			// set the global connection to NULL
			_conn = NULL;

			if (_running)
			{
				cout << "Connection to bundle daemon failed. Retry in " << backoff << " seconds." << endl;
				ibrcommon::Thread::sleep(backoff * 1000);

				// if backoff < 10 minutes
				if (backoff < 600)
				{
					// set a new backoff
					backoff = backoff * 2;
				}
			}
		} catch (const ibrcommon::IOException&)
		{
			// set the global connection to NULL
			_conn = NULL;

			if (_running)
			{
				cout << "Connection to bundle daemon failed. Retry in " << backoff << " seconds." << endl;
				ibrcommon::Thread::sleep(backoff * 1000);

				// if backoff < 10 minutes
				if (backoff < 600)
				{
					// set a new backoff
					backoff = backoff * 2;
				}
			}
		} catch (const std::exception&)
		{
			// set the global connection to NULL
			_conn = NULL;
		}
	}

	return (EXIT_SUCCESS);
}
