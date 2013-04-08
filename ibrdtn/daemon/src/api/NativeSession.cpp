/*
 * NativeSession.cpp
 *
 * Copyright (C) 2013 IBR, TU Braunschweig
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

#include "api/NativeSession.h"
#include "api/NativeSerializer.h"
#include "core/BundleCore.h"
#include "core/EventDispatcher.h"
#include "routing/QueueBundleEvent.h"

#include <ibrdtn/data/PayloadBlock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace api
	{
		NativeSessionCallback::~NativeSessionCallback()
		{
		}

		const std::string NativeSession::TAG = "NativeSession";

		NativeSession::NativeSession(NativeSessionCallback *cb)
		 : _receiver(*this), _cb(cb), _registration(dtn::core::BundleCore::getInstance().getSeeker()), _destroyed(false)
		{
			// set the local endpoint to the default
			_endpoint = _registration.getDefaultEID();

			// start the receiver
			_receiver.start();

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 15) << "Session created" << IBRCOMMON_LOGGER_ENDL;
		}

		NativeSession::~NativeSession()
		{
			destroy();
		}

		void NativeSession::destroy() throw ()
		{
			if (_destroyed) return;

			// prevent future destroy calls
			_destroyed = true;

			// send stop signal to the receiver
			_receiver.stop();

			// wait here until the receiver has been stopped
			_receiver.join();

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 15) << "Session destroyed" << IBRCOMMON_LOGGER_ENDL;
		}

		const dtn::data::EID& NativeSession::getNodeEID() const throw ()
		{
			return dtn::core::BundleCore::local;
		}

		void NativeSession::fireNotificationBundle(const dtn::data::BundleID &id) const throw ()
		{
			if (_cb == NULL) return;
			_cb->notifyBundle(id);
		}

		void NativeSession::fireNotificationStatusReport(const dtn::data::StatusReportBlock &report) const throw ()
		{
			if (_cb == NULL) return;
			_cb->notifyStatusReport(report);
		}

		void NativeSession::fireNotificationCustodySignal(const dtn::data::CustodySignalBlock &custody) const throw ()
		{
			if (_cb == NULL) return;
			_cb->notifyCustodySignal(custody);
		}

		void NativeSession::setEndpoint(const std::string &suffix) throw (NativeSessionException)
		{
			const dtn::data::EID new_endpoint = dtn::core::BundleCore::local + dtn::core::BundleCore::local.getDelimiter() + suffix;

			// error checking
			if (new_endpoint == dtn::data::EID())
			{
				throw NativeSessionException("given endpoint is not acceptable");
			}
			else
			{
				/* unsubscribe from the old endpoint and subscribe to the new one */
				_registration.unsubscribe(_endpoint);
				_registration.subscribe(new_endpoint);
				_endpoint = new_endpoint;
			}

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Endpoint set to " << _endpoint.getString() << IBRCOMMON_LOGGER_ENDL;
		}

		void NativeSession::resetEndpoint() throw ()
		{
			_registration.unsubscribe(_endpoint);
			_endpoint = _registration.getDefaultEID();
			_registration.subscribe(_endpoint);

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Endpoint set to " << _endpoint.getString() << IBRCOMMON_LOGGER_ENDL;
		}

		void NativeSession::addEndpoint(const std::string &suffix) throw (NativeSessionException)
		{
			const dtn::data::EID new_endpoint = dtn::core::BundleCore::local + dtn::core::BundleCore::local.getDelimiter() + suffix;

			// error checking
			if (new_endpoint == dtn::data::EID())
			{
				throw NativeSessionException("given endpoint is not acceptable");
			}
			else
			{
				_registration.subscribe(new_endpoint);
			}

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Endpoint " << suffix << " added" << IBRCOMMON_LOGGER_ENDL;
		}

		void NativeSession::removeEndpoint(const std::string &suffix) throw (NativeSessionException)
		{
			const dtn::data::EID old_endpoint = dtn::core::BundleCore::local + dtn::core::BundleCore::local.getDelimiter() + suffix;

			// error checking
			if (old_endpoint == dtn::data::EID())
			{
				throw NativeSessionException("given endpoint is not acceptable");
			}
			else
			{
				_registration.unsubscribe(old_endpoint);
			}

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Endpoint " << suffix << " removed" << IBRCOMMON_LOGGER_ENDL;
		}

		void NativeSession::addRegistration(const dtn::data::EID &eid) throw (NativeSessionException)
		{
			// error checking
			if (eid == dtn::data::EID())
			{
				throw NativeSessionException("given endpoint is not acceptable");
			}
			else
			{
				_registration.subscribe(eid);
			}

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Registration " << eid.getString() << " added" << IBRCOMMON_LOGGER_ENDL;
		}

		void NativeSession::removeRegistration(const dtn::data::EID &eid) throw (NativeSessionException)
		{
			// error checking
			if (eid == dtn::data::EID())
			{
				throw NativeSessionException("given endpoint is not acceptable");
			}
			else
			{
				_registration.unsubscribe(eid);
			}

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Registration " << eid.getString() << " removed" << IBRCOMMON_LOGGER_ENDL;
		}

		void NativeSession::clearRegistration() throw ()
		{
			resetEndpoint();

			const std::set<dtn::data::EID> subs = _registration.getSubscriptions();
			for (std::set<dtn::data::EID>::const_iterator it = subs.begin(); it != subs.end(); ++it) {
				const dtn::data::EID &e = (*it);
				if (e != _endpoint) {
					_registration.unsubscribe(e);
				}
			}

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Registrations cleared" << IBRCOMMON_LOGGER_ENDL;
		}

		std::vector<std::string> NativeSession::getSubscriptions() throw ()
		{
			const std::set<dtn::data::EID> subs = _registration.getSubscriptions();
			std::vector<std::string> ret;
			for (std::set<dtn::data::EID>::const_iterator it = subs.begin(); it != subs.end(); ++it) {
				ret.push_back((*it).getString());
			}
			return ret;
		}

		void NativeSession::next(RegisterIndex ri) throw (BundleNotFoundException)
		{
			try {
				const dtn::data::BundleID id = _bundle_queue.getnpop();

				IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Next bundle in queue is " << id.toString() << IBRCOMMON_LOGGER_ENDL;

				load(ri, id);
			} catch (const ibrcommon::QueueUnblockedException &ex) {
				IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 15) << "Failed to load bundle " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				throw BundleNotFoundException();
			}
		}

		void NativeSession::load(RegisterIndex ri, const dtn::data::BundleID &id) throw (BundleNotFoundException)
		{
			// load the bundle
			try {
				_bundle[ri] = dtn::core::BundleCore::getInstance().getStorage().get(id);

				// process the bundle block (security, compression, ...)
				dtn::core::BundleCore::processBlocks(_bundle[ri]);

				IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Bundle " << id.toString() << " loaded" << IBRCOMMON_LOGGER_ENDL;
			} catch (const ibrcommon::Exception &ex) {
				IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 15) << "Failed to load bundle " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				throw BundleNotFoundException();
			}
		}

		const dtn::data::Bundle& NativeSession::get(RegisterIndex ri) const throw ()
		{
			return _bundle[ri];
		}

		void NativeSession::get(RegisterIndex ri, NativeSerializerCallback &cb) const throw ()
		{
			NativeSerializer serializer(cb, NativeSerializer::BUNDLE_FULL);
			serializer << _bundle[ri];
		}

		void NativeSession::getInfo(RegisterIndex ri, NativeSerializerCallback &cb) const throw ()
		{
			NativeSerializer serializer(cb, NativeSerializer::BUNDLE_INFO);
			serializer << _bundle[ri];
		}

		void NativeSession::free(RegisterIndex ri) throw (BundleNotFoundException)
		{
			try {
				dtn::core::BundleCore::getInstance().getStorage().remove(_bundle[ri]);
				_bundle[ri] = dtn::data::Bundle();
			} catch (const ibrcommon::Exception&) {
				throw BundleNotFoundException();
			}
		}

		void NativeSession::clear(RegisterIndex ri) throw ()
		{
			_bundle[ri] = dtn::data::Bundle();
		}

		void NativeSession::delivered(const dtn::data::BundleID &id) throw (BundleNotFoundException)
		{
			try {
				// announce this bundle as delivered
				dtn::data::MetaBundle meta = dtn::core::BundleCore::getInstance().getStorage().get(id);
				_registration.delivered(meta);

				IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Bundle " << id.toString() << " marked as delivered" << IBRCOMMON_LOGGER_ENDL;
			} catch (const ibrcommon::Exception&) {
				throw BundleNotFoundException();
			}
		}

		void NativeSession::send(RegisterIndex ri) throw ()
		{
			// create a new sequence number
			_bundle[ri].relabel();

			// forward the bundle to the storage processing
			dtn::api::Registration::processIncomingBundle(_endpoint, _bundle[ri]);

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "Bundle " << _bundle[ri].toString() << " sent" << IBRCOMMON_LOGGER_ENDL;
		}

		void NativeSession::put(RegisterIndex ri, const dtn::data::Bundle &b) throw ()
		{
			// Copy the given bundle into the local register
			_bundle[ri] = b;
		}

		void NativeSession::put(RegisterIndex ri, const dtn::data::PrimaryBlock &p) throw ()
		{
			// clear all blocks in the register
			_bundle[ri].clear();

			// Copy the given primary block into the local register
			((dtn::data::PrimaryBlock&)_bundle[ri]) = p;
		}

		void NativeSession::write(RegisterIndex ri, const char *buf, const size_t len, const size_t offset) throw ()
		{
			try {
				dtn::data::PayloadBlock &payload = _bundle[ri].find<dtn::data::PayloadBlock>();

				ibrcommon::BLOB::Reference ref = payload.getBLOB();
				ibrcommon::BLOB::iostream stream = ref.iostream();

				ssize_t stream_size = stream.size();

				if ((offset > 0) || (stream_size < offset)) {
					(*stream).seekp(0, std::ios_base::end);
				} else {
					(*stream).seekp(offset);
				}

				(*stream).write(buf, len);
				(*stream) << std::flush;
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
				dtn::data::PayloadBlock &payload = _bundle[ri].push_back<dtn::data::PayloadBlock>();

				ibrcommon::BLOB::Reference ref = payload.getBLOB();
				ibrcommon::BLOB::iostream stream = ref.iostream();

				ssize_t stream_size = stream.size();

				if ((offset > 0) || (stream_size < offset)) {
					(*stream).seekp(0, std::ios_base::end);
				} else {
					(*stream).seekp(offset);
				}
				(*stream).write(buf, len);
				(*stream) << std::flush;
			}

			IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 25) << len << " bytes added to the payload" << IBRCOMMON_LOGGER_ENDL;
		}

		void NativeSession::read(RegisterIndex ri, char *buf, size_t &len, const size_t offset) throw ()
		{
			try {
				dtn::data::PayloadBlock &payload = _bundle[ri].find<dtn::data::PayloadBlock>();

				ibrcommon::BLOB::Reference ref = payload.getBLOB();
				ibrcommon::BLOB::iostream stream = ref.iostream();

				(*stream).seekg(offset);
				(*stream).read(buf, len);

				len = (*stream).gcount();
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
				IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 25) << "no payload block available" << IBRCOMMON_LOGGER_ENDL;
				len = 0;
			}
		}

		NativeSession::BundleReceiver::BundleReceiver(NativeSession &session)
		 : _session(session), _shutdown(false)
		{
		}

		NativeSession::BundleReceiver::~BundleReceiver()
		{
			ibrcommon::JoinableThread::join();
		}

		void NativeSession::BundleReceiver::raiseEvent(const dtn::core::Event *evt) throw ()
		{
			try {
				const dtn::routing::QueueBundleEvent &queued = dynamic_cast<const dtn::routing::QueueBundleEvent&>(*evt);

				// ignore fragments - we can not deliver them directly to the client
				if (queued.bundle.fragment) return;

				if (_session._registration.hasSubscribed(queued.bundle.destination))
				{
					_session._registration.notify(Registration::NOTIFY_BUNDLE_AVAILABLE);
				}
			} catch (const std::bad_cast&) { };
		}

		void NativeSession::BundleReceiver::__cancellation() throw ()
		{
			// set the shutdown variable to true
			_shutdown = true;

			// abort all blocking calls on the registration object
			_session._registration.abort();
		}

		void NativeSession::BundleReceiver::finally() throw ()
		{
		}

		void NativeSession::BundleReceiver::run() throw ()
		{
			// listen to QueueBundleEvents
			dtn::core::EventDispatcher<dtn::routing::QueueBundleEvent>::add(this);

			Registration &reg = _session._registration;
			try{
				while (!_shutdown) {
					try {
						const dtn::data::MetaBundle id = reg.receiveMetaBundle();

						if (id.procflags & dtn::data::PrimaryBlock::APPDATA_IS_ADMRECORD) {
							// transform custody signals & status reports into notifies
							fireNotificationAdministrativeRecord(id);

							// announce the delivery of this bundle
							reg.delivered(id);
						} else {
							IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "[BundleReceiver] fire notification for new bundle " << id.toString() << IBRCOMMON_LOGGER_ENDL;

							// put the bundle into the API queue
							_session._bundle_queue.push(id);

							// notify the client about the new bundle
							_session.fireNotificationBundle(id);
						}
					} catch (const dtn::storage::NoBundleFoundException&) {
						reg.wait_for_bundle();
					}

					yield();
				}
			} catch (const ibrcommon::QueueUnblockedException &ex) {
				IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 40) << "[BundleReceiver] " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			} catch (const std::exception &ex) {
				IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 10) << "[BundleReceiver] " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}

			// un-listen from QueueBundleEvents
			dtn::core::EventDispatcher<dtn::routing::QueueBundleEvent>::remove(this);
		}

		void NativeSession::BundleReceiver::fireNotificationAdministrativeRecord(const dtn::data::MetaBundle &bundle)
		{
			// load the whole bundle
			const dtn::data::Bundle b = dtn::core::BundleCore::getInstance().getStorage().get(bundle);

			// get the payload block of the bundle
			const dtn::data::PayloadBlock &payload = b.find<dtn::data::PayloadBlock>();

			try {
				// try to decode as status report
				dtn::data::StatusReportBlock report;
				report.read(payload);

				IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "fire notification for status report" << IBRCOMMON_LOGGER_ENDL;

				// fire the status report notification
				_session.fireNotificationStatusReport(report);
			} catch (const dtn::data::StatusReportBlock::WrongRecordException&) {
				// this is not a status report
			}

			try {
				// try to decode as custody signal
				dtn::data::CustodySignalBlock custody;
				custody.read(payload);

				IBRCOMMON_LOGGER_DEBUG_TAG(NativeSession::TAG, 20) << "fire notification for custody signal" << IBRCOMMON_LOGGER_ENDL;

				// fire the custody signal notification
				_session.fireNotificationCustodySignal(custody);
			} catch (const dtn::data::CustodySignalBlock::WrongRecordException&) {
				// this is not a custody report
			}
		}
	} /* namespace net */
} /* namespace dtn */
