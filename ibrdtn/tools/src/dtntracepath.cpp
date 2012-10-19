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
#include <ibrdtn/data/StatusReportBlock.h>
#include "ibrdtn/api/Client.h"
#include "ibrdtn/api/StringBundle.h"
#include <ibrcommon/net/socket.h>
#include <ibrcommon/net/socketstream.h>
#include "ibrcommon/thread/Mutex.h"
#include "ibrcommon/thread/MutexLock.h"
#include "ibrcommon/TimeMeasurement.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <unistd.h>

class ReportBundle : public dtn::api::Bundle
{
public:
	ReportBundle(const dtn::api::Bundle &apib)
	 : Bundle(apib) {
	}

	virtual ~ReportBundle() {

	}

	const dtn::data::StatusReportBlock& extract()
	{
		return _b.getBlock<dtn::data::StatusReportBlock>();
	}
};

class Tracer : public dtn::api::Client
{
	public:
		Tracer(dtn::api::Client::COMMUNICATION_MODE mode, ibrcommon::socketstream &stream)
		 : dtn::api::Client("", stream, mode), _stream(stream)
		{
		}

		virtual ~Tracer()
		{
		}

		void printReason(char code) {
			switch (code) {
			case dtn::data::StatusReportBlock::LIFETIME_EXPIRED:
				::printf("      * lifetime expired\n");
				break;
			case dtn::data::StatusReportBlock::FORWARDED_OVER_UNIDIRECTIONAL_LINK:
				::printf("      * forwarded over unidirectional link\n");
				break;
			case dtn::data::StatusReportBlock::TRANSMISSION_CANCELED:
				::printf("      * transmission canceled\n");
				break;
			case dtn::data::StatusReportBlock::DEPLETED_STORAGE:
				::printf("      * depleted storage\n");
				break;
			case dtn::data::StatusReportBlock::DESTINATION_ENDPOINT_ID_UNINTELLIGIBLE:
				::printf("      * destination endpoint ID is unintelligible\n");
				break;
			case dtn::data::StatusReportBlock::NO_KNOWN_ROUTE_TO_DESTINATION_FROM_HERE:
				::printf("      * no known route to destination from here\n");
				break;
			case dtn::data::StatusReportBlock::NO_TIMELY_CONTACT_WITH_NEXT_NODE_ON_ROUTE:
				::printf("      * no timely contact with next node on route\n");
				break;
			case dtn::data::StatusReportBlock::BLOCK_UNINTELLIGIBLE:
				::printf("      * block unintelligible\n");
				break;
			default:
				break;
			}
		}

		void formattedLog(size_t i, const dtn::api::Bundle &b, const ibrcommon::TimeMeasurement &tm)
		{
			ReportBundle report(b);
			const dtn::data::StatusReportBlock &sr = report.extract();

			const std::string source = b.getSource().getString();

			// format time data
			std::stringstream time;
			time << tm;

			if (sr._status & dtn::data::StatusReportBlock::FORWARDING_OF_BUNDLE) {
				std::stringstream type;
				type << "forwarded";
				::printf(" %3d: %-48s %-12s %6s\n", (unsigned int)i, source.c_str(), type.str().c_str(), time.str().c_str());
				printReason(sr._reasoncode);
			}

			if (sr._status & dtn::data::StatusReportBlock::RECEIPT_OF_BUNDLE) {
				std::stringstream type;
				type << "reception";
				::printf(" %3d: %-48s %-12s %6s\n", (unsigned int)i, source.c_str(), type.str().c_str(), time.str().c_str());
				printReason(sr._reasoncode);
			}

			if (sr._status & dtn::data::StatusReportBlock::DELIVERY_OF_BUNDLE) {
				std::stringstream type;
				type << "delivery";
				::printf(" %3d: %-48s %-12s %6s\n", (unsigned int)i, source.c_str(), type.str().c_str(), time.str().c_str());
				printReason(sr._reasoncode);
			}

			if (sr._status & dtn::data::StatusReportBlock::DELETION_OF_BUNDLE) {
				std::stringstream type;
				type << "deletion";
				::printf(" %3d: %-48s %-12s %6s\n", (unsigned int)i, source.c_str(), type.str().c_str(), time.str().c_str());
				printReason(sr._reasoncode);
			}

			::fflush(stdout);
		}

		void tracepath(const dtn::data::EID &destination, size_t timeout, unsigned char options)
		{
			// create a bundle
			dtn::api::StringBundle b(destination);

			// set lifetime
			b.setLifetime(timeout);

			// set some stupid payload
			b.append("follow the white rabbit");

			// request forward and delivery reports
			if (options & 0x01) b.requestForwardedReport();
			if (options & 0x02) b.requestDeliveredReport();
			if (options & 0x04) b.requestReceptionReport();
			if (options & 0x08) b.requestDeletedReport();

			b.setReportTo( EID("api:me") );

			ibrcommon::TimeMeasurement tm;

			std::list<dtn::api::Bundle> bundles;

			try {
				size_t i = 0;

				// start the timer
				tm.start();

				std::cout << "TRACE TO " << destination.getString() << std::endl;

				// send the bundle
				(*this) << b; (*this).flush();

				try {
					// now receive all incoming bundles
					while (true)
					{
						dtn::api::Bundle recv = getBundle(timeout);
						bundles.push_back(recv);
						tm.stop();

						// generate formatted output
						i++;
						formattedLog(i, recv, tm);
					}
				} catch (const std::exception&) {

				}

				std::cout << std::endl;

//				// sort the list
//				bundles.sort();
//
//				// print out each hop
//				for (std::list<dtn::api::Bundle>::iterator iter = bundles.begin(); iter != bundles.end(); iter++)
//				{
//					const dtn::api::Bundle &b = (*iter);
//
//					ReportBundle report(b);
//					const dtn::data::StatusReportBlock &sr = report.extract();
//
//
//					std::cout << "Hop: " << (*iter).getSource().getString() << std::endl;
//
//					if (sr._status & dtn::data::StatusReportBlock::FORWARDING_OF_BUNDLE) {
//						std::cout << "   bundle forwarded: " << sr._timeof_forwarding.getTimestamp().getValue() << std::endl;
//					}
//
//					if (sr._status & dtn::data::StatusReportBlock::RECEIPT_OF_BUNDLE) {
//						std::cout << "   bundle reception: " << sr._timeof_receipt.getTimestamp().getValue() << std::endl;
//					}
//
//					if (sr._status & dtn::data::StatusReportBlock::DELIVERY_OF_BUNDLE) {
//						std::cout << "   bundle delivery: " << sr._timeof_delivery.getTimestamp().getValue() << std::endl;
//					}
//
//					if (sr._status & dtn::data::StatusReportBlock::DELETION_OF_BUNDLE) {
//						std::cout << "   bundle deletion: " << sr._timeof_deletion.getTimestamp().getValue() << std::endl;
//					}
//				}

			} catch (const dtn::api::ConnectionException&) {
				cout << "Disconnected." << endl;
			} catch (const ibrcommon::IOException&) {
				cout << "Error while receiving a bundle." << endl;
			}
		}

	private:
		ibrcommon::socketstream &_stream;
};

void print_help()
{
	cout << "-- dtntracepath (IBR-DTN) --" << endl;
	cout << "Syntax: dtntracepath [options] <dst>"  << endl;
	cout << " <dst>    set the destination eid (e.g. dtn://node/null)" << endl;
	cout << "* optional parameters *" << endl;
	cout << " -h              display this text" << endl;
	cout << " -t <timeout>    time in seconds to wait for reports" << endl;
	cout << " -d              request deletion report" << endl;
	cout << " -f              request forward report" << endl;
	cout << " -r              request reception report" << endl;
}

int main(int argc, char *argv[])
{
	std::string trace_destination = "dtn://local";
	// forwarded = 0x01
	// delivered = 0x02
	// reception = 0x04
	// deletion = 0x08
	unsigned char report_options = 0x02;

	size_t timeout = 10;
	int opt = 0;

	dtn::api::Client::COMMUNICATION_MODE mode = dtn::api::Client::MODE_BIDIRECTIONAL;

	if (argc == 1)
	{
		print_help();
		return 0;
	}

	while ((opt = getopt(argc, argv, "ht:fdr")) != -1)
	{
		switch (opt)
		{
		case 'h':
			print_help();
			return 0;

		case 't':
			timeout = atoi(optarg);
			break;

		case 'f':
			report_options |= 0x01;
			break;

		case 'd':
			report_options |= 0x08;
			break;

		case 'r':
			report_options |= 0x04;
			break;
		}
	}

	// the last parameter is always the destination
	trace_destination = argv[argc - 1];

	try {
		// Create a stream to the server using TCP.
		ibrcommon::vaddress addr("localhost");
		ibrcommon::socketstream conn(new ibrcommon::tcpsocket(addr, 4550));

		// Initiate a derivated client
		Tracer tracer(mode, conn);

		// Connect to the server. Actually, this function initiate the
		// stream protocol by starting the thread and sending the contact header.
		tracer.connect();

		// target address
		tracer.tracepath(trace_destination, timeout, report_options);

		// Shutdown the client connection.
		tracer.close();
		conn.close();

	} catch (const ibrcommon::socket_exception&) {
		std::cerr << "Can not connect to the daemon. Does it run?" << std::endl;
		return -1;
	} catch (const std::exception &e) {
		std::cerr << "Unexcepted error: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}
