#include "net/LOWPANConnection.h"
#include "net/BundleReceivedEvent.h"
#include "core/BundleEvent.h"
#include "net/TransferCompletedEvent.h"
#include "net/TransferAbortedEvent.h"
#include "routing/RequeueBundleEvent.h"
#include "core/BundleCore.h"

#include <ibrcommon/Logger.h>
#include <ibrcommon/thread/MutexLock.h>

#include <ibrdtn/data/ScopeControlHopLimitBlock.h>
#include <ibrdtn/utils/Utils.h>
#include <ibrdtn/data/Serializer.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <iostream>
#include <list>

using namespace dtn::data;

namespace dtn
{
	namespace net
	{
		LOWPANConnection::LOWPANConnection(unsigned short address, LOWPANConvergenceLayer &cl)
			: _address(address), _stream(cl, address), _sender(_stream), _cl(cl)
		{
		}

		LOWPANConnection::~LOWPANConnection()
		{
		}

		ibrcommon::lowpanstream& LOWPANConnection::getStream()
		{
			return _stream;
		}

		void LOWPANConnection::setup()
		{
			_sender.start();
		}

		void LOWPANConnection::finally()
		{
			_sender.stop();
			_sender.join();

			// remove this connection from the connection list
			_cl.remove(this);
		}

		void LOWPANConnection::run()
		{
			try {
				while(_stream.good())
				{
					try {
						dtn::data::DefaultDeserializer deserializer(_stream);
						dtn::data::Bundle bundle;
						deserializer >> bundle;

						IBRCOMMON_LOGGER_DEBUG(10) << "LOWPANConnection::run"<< IBRCOMMON_LOGGER_ENDL;

						// determine sender
						EID sender;

						// increment value in the scope control hop limit block
						try {
							dtn::data::ScopeControlHopLimitBlock &schl = bundle.getBlock<dtn::data::ScopeControlHopLimitBlock>();
							schl.increment();
						} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };

						// raise default bundle received event
						dtn::net::BundleReceivedEvent::raise(sender, bundle, false, true);

					} catch (const dtn::InvalidDataException &ex) {
						IBRCOMMON_LOGGER_DEBUG(10) << "Received a invalid bundle: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					} catch (const ibrcommon::IOException &ex) {
						IBRCOMMON_LOGGER_DEBUG(10) << "IOException: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
				}
			} catch (std::exception &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "Thread died: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void LOWPANConnection::__cancellation()
		{
			_sender.stop();
		}

		// class LOWPANConnectionSender
		LOWPANConnectionSender::LOWPANConnectionSender(ibrcommon::lowpanstream &stream)
		: _stream(stream)
		{
		}

		LOWPANConnectionSender::~LOWPANConnectionSender()
		{
		}

		void LOWPANConnectionSender::queue(const ConvergenceLayer::Job &job)
		{
			IBRCOMMON_LOGGER_DEBUG(10) << ":LOWPANConnectionSender::queue"<< IBRCOMMON_LOGGER_ENDL;
			_queue.push(job);
		}

		void LOWPANConnectionSender::run()
		{
			try {
				while(_stream.good())
				{
					ConvergenceLayer::Job job = _queue.getnpop(true);
					dtn::data::DefaultSerializer serializer(_stream);

					IBRCOMMON_LOGGER_DEBUG(10) << ":LOWPANConnectionSender::run"<< IBRCOMMON_LOGGER_ENDL;

					dtn::storage::BundleStorage &storage = dtn::core::BundleCore::getInstance().getStorage();

					// read the bundle out of the storage
					const dtn::data::Bundle bundle = storage.get(job._bundle);

					// Put bundle into stringstream
					serializer << bundle; _stream.flush();
					// raise bundle event
					dtn::net::TransferCompletedEvent::raise(job._destination, bundle);
					dtn::core::BundleEvent::raise(bundle, dtn::core::BUNDLE_FORWARDED);
				}
				// FIXME: Exit strategy when sending on socket failed. Like destroying the connection object
				// Also check what needs to be done when the node is not reachable (transfer requeue...)

				IBRCOMMON_LOGGER_DEBUG(10) << ":LOWPANConnectionSender::run stream destroyed"<< IBRCOMMON_LOGGER_ENDL;
			} catch (std::exception &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "Thread died: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void LOWPANConnectionSender::__cancellation()
		{
			_queue.abort();
		}
	}
}
