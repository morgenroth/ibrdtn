/*
 * dtnconvert.cpp
 *
 *  Created on: 11.01.2011
 *      Author: morgenro
 */

#include "config.h"

#include <ibrdtn/data/Serializer.h>
#include <ibrdtn/data/PayloadBlock.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/EID.h>

#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/data/File.h>

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <unistd.h>

void print_help()
{
	std::cout << "-- dtnconvert (IBR-DTN) --" << std::endl;
	std::cout << "Syntax: dtnconvert [options]"  << std::endl;
	std::cout << std::endl;
	std::cout << "* parameters *" << std::endl;
	std::cout << " -h        display this text" << std::endl;
	std::cout << " -r        read bundle data and print out some information" << std::endl;
	std::cout << " -c        create a bundle with stdin as payload" << std::endl;
	std::cout << std::endl;
	std::cout << "* options when creating a bundle *" << std::endl;
	std::cout << " -s        source EID of the bundle" << std::endl;
	std::cout << " -d        destination EID of the bundle" << std::endl;
	std::cout << " -l        lifetime of the bundle in seconds (default: 3600)" << std::endl;
}

int main(int argc, char** argv)
{
	int opt = 0;
	int working_mode = 0;
	dtn::data::EID _source;
	dtn::data::EID _destination;
	size_t _lifetime = 3600;

	while((opt = getopt(argc, argv, "hrcs:d:l:")) != -1)
	{
		switch (opt)
		{
		case 'h':
			print_help();
			break;

		case 'r':
			working_mode = 1;
			break;

		case 'c':
			working_mode = 2;
			break;

		case 'l':
			_lifetime = atoi(optarg);
			break;

		case 's':
			_source = std::string(optarg);
			break;

		case 'd':
			_destination = std::string(optarg);
			break;

		default:
			std::cout << "unknown command" << std::endl;
			return -1;
		}
	}

	if (working_mode == 0)
	{
		print_help();
		return -1;
	}

	switch (working_mode)
	{
		case 1:
		{
			dtn::data::DefaultDeserializer dd(std::cin);
			dtn::data::Bundle b;
			dd >> b;

			std::cout << "flags: " << std::hex << std::setw( 2 ) << std::setfill( '0' ) << b._procflags << std::dec << std::endl;
			std::cout << "source: " << b._source.getString() << std::endl;
			std::cout << "destination: " << b._destination.getString() << std::endl;
			std::cout << "timestamp: " << b._timestamp << std::endl;
			std::cout << "sequence number: " << b._sequencenumber << std::endl;
			std::cout << "lifetime: " << b._lifetime << std::endl;

			const dtn::data::PayloadBlock &pblock = b.getBlock<dtn::data::PayloadBlock>();
			ibrcommon::BLOB::Reference ref = pblock.getBLOB();

			// this part is protected agains other threads
			{
				ibrcommon::BLOB::iostream stream = ref.iostream();
				std::cout << "payload size: " << stream.size() << std::endl;
			}
			break;
		}

		case 2:
		{
			dtn::data::DefaultSerializer ds(std::cout);
			dtn::data::Bundle b;

			b._source = _source;
			b._destination = _destination;
			b._lifetime = _lifetime;

			const dtn::data::PayloadBlock &pblock = b.push_back<dtn::data::PayloadBlock>();
			ibrcommon::BLOB::Reference ref = pblock.getBLOB();

			// this part is protected agains other threads
			{
				ibrcommon::BLOB::iostream stream = ref.iostream();
				(*stream) << std::cin.rdbuf();
			}

			ds << b;
			break;
		}
	}

	return 0;
}
