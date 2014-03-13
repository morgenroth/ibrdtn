/*
 * dtnoutbox.cpp
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 * 			   David Goltzsche <goltzsch@ibr.cs.tu-bs.de>
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
#include <ibrdtn/api/Client.h>
#include <ibrcommon/net/socket.h>
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/thread/SignalHandler.h>
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/data/File.h>
#include <ibrcommon/appstreambuf.h>

#include "io/TarUtils.h"
#include "io/ObservedNormalFile.h"

#ifdef HAVE_LIBTFFS
#include "io/ObservedFATFile.h"
extern "C"
{
#include <tffs.h>
}
#endif

#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <regex.h>
#include <getopt.h>

// set this variable to false to stop the app
bool _running = true;

//global wait conditional
ibrcommon::Conditional _wait_cond;
bool _wait_abort = false;

//global conf values
string _conf_name;
string _conf_outbox;
string _conf_destination;

//optional paramters
string _conf_workdir;
size_t _conf_interval = 5000;
size_t _conf_rounds = 3;
string _conf_path = "/";
string _conf_regex_str = "^\\.";
regex_t _conf_regex;
int _conf_bundle_group = false;
int _conf_invert = false;
int _conf_quiet = false;
int _conf_fat = false;
int _conf_enabled = true;

struct option long_options[] =
{
	{"destination",  required_argument, 0, 'd'},
	{"help", no_argument, 0, 'h'},
	{"group", no_argument, 0, 'g'},
	{"workdir", required_argument, 0, 'w'},
	{"interval", required_argument, 0, 'i'},
	{"rounds", required_argument, 0, 'r'},
	{"path", required_argument, 0, 'p'},
	{"regex", required_argument, 0, 'R'},
	{"quiet", no_argument, 0, 'q'},
	{0, 0, 0, 0}
};
void print_help()
{
	std::cout << "-- dtnoutbox (IBR-DTN) --" << std::endl;
	std::cout << "Syntax: dtnoutbox [options] <name> <outbox> <destination>" << std::endl;
	std::cout << " <name>           The application name" << std::endl;
#ifdef HAVE_LIBTFFS
	std::cout << " <outbox>         Location of outgoing files, directory or vfat-image" << std::endl;
#else
	std::cout << " <outbox>         Directory of outgoing files" << std::endl;
#endif
	std::cout << " <destination>    The destination EID for all outgoing files" << std::endl << std::endl;
	std::cout << "* optional parameters *" << std::endl;
	std::cout << " -h|--help        Display this text" << std::endl;
	std::cout << " -g|--group       Receiver is a destination group" << std::endl;
	std::cout << " -w|--workdir <dir>" << std::endl;
	std::cout << "                  Temporary work directory" << std::endl;
	std::cout << " -i|--interval <milliseconds>" << std::endl;
	std::cout << "                  Interval in milliseconds, in which <outbox> is scanned" << std::endl;
	std::cout << "                  for new/changed files. default: 5000" << std::endl;
	std::cout << " -r|--rounds <n>  Number of rounds of intervals, after which a unchanged" << std::endl;
	std::cout << "                  file is considered as written. default: 3" << std::endl;
#ifdef HAVE_LIBTFFS
	std::cout << " -p|--path <path> Path of outbox within vfat-image. default: /" << std::endl;
#endif
	std::cout << " -R|--regex <regex>" << std::endl;
	std::cout << "                  All files in <outbox> matching this regular expression" << std::endl;
	std::cout << "                  will be ignored. default: ^\\." << std::endl;
	std::cout << " -I|--invert      Invert the regular expression defined with -R"<< std::endl;
	std::cout << " -q|--quiet       Only print error messages" << std::endl;

	_running = false; //stop this app, after printing help

}

void read_configuration(int argc, char** argv)
{
	// print help if not enough parameters are set
	if (argc < 3)
	{
		print_help();
		exit(EXIT_FAILURE);
	}

	while(1)
	{
		/* getopt_long stores the option index here. */
		int option_index = 0;
		int c = getopt_long (argc, argv, "hw:i:r:p:R:qIg",
				long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
		case 0:
			/* If this option set a flag, do nothing else now. */
			if (long_options[option_index].flag != 0)
				break;
			printf ("option %s", long_options[option_index].name);
			if (optarg)
				printf (" with arg %s", optarg);
			printf ("\n");
			break;

		case 'h':
			print_help();
			exit(EXIT_SUCCESS);
			break;
		case 'g':
			_conf_bundle_group = true;
			break;
		case 'w':
			_conf_workdir = std::string(optarg);
			break;
		case 'i':
			_conf_interval = atoi(optarg);
			break;
		case 'r':
			_conf_rounds = atoi(optarg);
			break;
		case 'p':
			_conf_path = std::string(optarg);
			break;
		case 'R':
			_conf_regex_str = std::string(optarg);
			break;
		case 'q':
			_conf_quiet = true;
			break;
		case 'I':
			_conf_invert = true;
			break;
		case '?':
			break;
		default:
			abort();
			break;
		}
	}

	_conf_name = std::string(argv[optind]);
	_conf_destination = std::string(argv[optind+2]);
	_conf_outbox = std::string(argv[optind+1]);

	//compile regex, if set
	if (_conf_regex_str.length() > 0 && regcomp(&_conf_regex,_conf_regex_str.c_str(),0))
	{
		std::cout << "ERROR: invalid regex: " << optarg << std::endl;
		exit(-1);
	}


	//check outbox path for trailing slash
	if (_conf_outbox.at(_conf_outbox.length()-1) == '/')
		_conf_outbox = _conf_outbox.substr(0,_conf_outbox.length() -1);
}

void sighandler_func(int signal)
{
	switch (signal)
	{
	case SIGTERM:
	case SIGINT:
	{
		//stop waiting and stop running, on SIGINT or SIGTERM
		ibrcommon::MutexLock l(_wait_cond);
		_running = false;
		_wait_cond.signal(true);
		break;
	}
#ifndef __WIN32__
	case SIGUSR1:
	{
		//stop waiting on SIGUSR1 -> "quickscan"
		ibrcommon::MutexLock l(_wait_cond);
		_wait_abort = true;
		_wait_cond.signal(true);
		break;
	}
#endif
	default:
		break;
	}
}

//this function is needed, to delete the pointers in the list observed_files
bool deleteAll(io::ObservedFile* ptr){
	delete ptr;
	return true;
}

bool path_equals( io::ObservedFile *a, io::ObservedFile *b )
{
	return (a->getPath() != b->getPath());
}

/*
 * main application method
 */
int main( int argc, char** argv )
{
	// catch process signals
	ibrcommon::SignalHandler sighandler(sighandler_func);
	sighandler.handle(SIGINT);
	sighandler.handle(SIGTERM);
#ifndef __WIN32__
    sighandler.handle(SIGUSR1);
#endif

	// read the configuration
	read_configuration(argc,argv);

	// initialize sighandler after possible exit call
	sighandler.initialize();

	// init working directory
	if (_conf_workdir.length() > 0)
	{
		ibrcommon::File blob_path(_conf_workdir);

		if (blob_path.exists())
		{
			ibrcommon::BLOB::changeProvider(new ibrcommon::FileBLOBProvider(blob_path), true);
		}
	}

	// backoff for reconnect
	unsigned int backoff = 2;

	ibrcommon::File outbox_file(_conf_outbox);

	typedef std::list<io::ObservedFile*> filelist;
	filelist avail_files, new_files, old_files, deleted_files, observed_files, files_to_send;

	typedef std::set<io::FileHash> hashlist;
	hashlist sent_hashes;

	io::ObservedFile* outbox;

	if (outbox_file.exists() && !outbox_file.isDirectory())
	{
#ifdef HAVE_LIBTFFS
		_conf_fat = true;
		outbox = new io::ObservedFATFile(_conf_outbox, _conf_path);
#else
		std::cout << "ERROR: image-file provided, but this tool has been compiled without libtffs support!" << std::endl;
		return -1;
#endif
	}
	else
	{
		if (!outbox_file.exists())
			ibrcommon::File::createDirectory(outbox_file);

		outbox = new io::ObservedNormalFile(_conf_outbox);
	}

	if (!_conf_quiet) std::cout << "-- dtnoutbox --" << std::endl;

	// loop, if no stop if requested
	while (_running)
	{
		try
		{
			// Create a stream to the server using TCP.
			ibrcommon::vaddress addr("localhost", 4550);
			ibrcommon::socketstream conn(new ibrcommon::tcpsocket(addr));

			// Initiate a client for synchronous receiving
			dtn::api::Client client(_conf_name, conn, dtn::api::Client::MODE_SENDONLY);

			// Connect to the server. Actually, this function initiate the
			// stream protocol by starting the thread and sending the contact header.
			client.connect();

			// reset backoff if connected
			backoff = 2;

			// check the connection
			while (_running)
			{
				deleted_files.clear();
				new_files.clear();
				old_files.clear();

				// store old files
				old_files = avail_files;
				avail_files.clear();

				// get all files
				outbox->getFiles(avail_files);

				// determine deleted files
				std::set_difference(old_files.begin(), old_files.end(), avail_files.begin(), avail_files.end(), std::back_inserter(deleted_files), path_equals);

				// remove deleted files from observation
				for (filelist::iterator iter = deleted_files.begin(); iter != deleted_files.end(); ++iter)
				{
					sent_hashes.erase((*iter)->getHash());
					for(filelist::iterator iter2 = observed_files.begin();iter2 != observed_files.end();++iter2)
					{
							if ((*iter2)->getHash() == (*iter)->getHash())
							{
								delete (*iter);
								observed_files.erase(iter2);
								break;
							}
					}
				}

				// re-run loop, if files have been deleted
				if (deleted_files.size() > 0) break;

				// determine new files
				std::set_difference(avail_files.begin(), avail_files.end(), old_files.begin(), old_files.end(), std::back_inserter(new_files), path_equals);

				// add new files to observation
				for (filelist::iterator iter = new_files.begin(); iter != new_files.end(); ++iter)
				{
					const io::ObservedFile &of = (**iter);

					int reg_ret = regexec(&_conf_regex,(*iter)->getBasename().c_str(), 0, NULL, 0);
					if (!reg_ret && !_conf_invert)
						continue;
					if (reg_ret && _conf_invert)
						continue;

					// print error message, if regex error occurs
					if (reg_ret && reg_ret != REG_NOMATCH)
					{
							char msgbuf[100];
							regerror(reg_ret,&_conf_regex,msgbuf,sizeof(msgbuf));
							std::cerr << "ERROR: regex match failed : " << std::string(msgbuf) << std::endl;
					}

					// log output
					if (!_conf_quiet) std::cout << "file found: " << of.getBasename() << std::endl;

					if (!_conf_fat) {
						observed_files.push_back( new io::ObservedNormalFile(of.getPath()) );
#ifdef HAVE_LIBTFFS
					} else {
						observed_files.push_back( new io::ObservedFATFile(_conf_outbox, of.getPath()) );
#endif
					}
				}

				// tick and update all files
				for (filelist::iterator iter = observed_files.begin(); iter != observed_files.end(); ++iter)
				{
					(*iter)->update();
					(*iter)->tick();
				}

				// find files to send, create std::list
				files_to_send.clear();
				for (filelist::iterator iter = observed_files.begin(); iter != observed_files.end(); ++iter)
				{
					io::ObservedFile &of = (**iter);

					if (sent_hashes.find(of.getHash()) == sent_hashes.end())
					{
						if (of.lastHashesEqual(_conf_rounds))
						{
							of.send();
							sent_hashes.insert(of.getHash());
							files_to_send.push_back(*iter);
						}
					}
				}

				if (!files_to_send.empty())
				{
					if (!_conf_quiet)
					{
						std::cout << "send files: ";
						for (filelist::const_iterator it = files_to_send.begin(); it != files_to_send.end(); ++it) {
							std::cout << (*it)->getBasename() << " ";
						}
						std::cout << std::endl;
					}

					// create a blob
					ibrcommon::BLOB::Reference blob = ibrcommon::BLOB::create();

					// set paths, depends on file location (normal file system or fat image)
					if (_conf_fat)
					{
						// write files into BLOB while it is locked
						ibrcommon::BLOB::iostream stream = blob.iostream();
						io::TarUtils::write(*stream, _conf_path, files_to_send);
					}
					else
					{
						// write files into BLOB while it is locked
						ibrcommon::BLOB::iostream stream = blob.iostream();
						io::TarUtils::write(*stream, _conf_outbox, files_to_send);
					}

					// create a new bundle
					dtn::data::EID destination = EID(_conf_destination);

					// create a new bundle
					dtn::data::Bundle b;

					// set destination
					b.destination = destination;

					// add payload block using the blob
					b.push_back(blob);

					// set destination address to non-singleton, if configured
					if (_conf_bundle_group)
						b.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, false);


					// send the bundle
					client << b;
					client.flush();
				}

				if (_running)
				{
					// wait defined seconds
					ibrcommon::MutexLock l(_wait_cond);
					_wait_cond.wait(_conf_interval);
					if (_wait_abort)
						_wait_abort = false;
				}
			}

			// clean up pointer list
			observed_files.remove_if(deleteAll);

			// clean up regex
			regfree(&_conf_regex);

			// close the client connection
			client.close();

			// close the connection
			conn.close();

		}
		catch (const ibrcommon::socket_exception&)
		{
			if (_running)
			{
				std::cout << "Connection to bundle daemon failed. Retry in " << backoff << " seconds." << std::endl;
				ibrcommon::Thread::sleep(backoff * 1000);

				// if backoff < 10 minutes
				if (backoff < 600)
				{
					// set a new backoff
					backoff = backoff * 2;
				}
			}
		}
		catch (const ibrcommon::IOException&)
		{
			if (_running)
			{
				std::cout << "Connection to bundle daemon failed. Retry in " << backoff << " seconds." << std::endl;
				ibrcommon::Thread::sleep(backoff * 1000);

				// if backoff < 10 minutes
				if (backoff < 600)
				{
					// set a new backoff
					backoff = backoff * 2;
				}
			}
		}
		catch (const std::exception&) { };
	}

	return (EXIT_SUCCESS);
}
