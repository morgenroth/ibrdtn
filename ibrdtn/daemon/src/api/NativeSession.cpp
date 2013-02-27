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

#include "NativeSession.h"
#include "core/BundleCore.h"

#include <ibrdtn/data/PayloadBlock.h>
#include <ibrcommon/Logger.h>

namespace dtn
{
	namespace api
	{
		NativeSessionCallback::~NativeSessionCallback()
		{
		}

		NativeSession::NativeSession(NativeSessionCallback *cb)
		 : _receiver(*this), _cb(cb), _registration(dtn::core::BundleCore::getInstance().getSeeker())
		{
			// set the local endpoint to the default
			_endpoint = _registration.getDefaultEID();

			// start the receiver
			_receiver.start();

			IBRCOMMON_LOGGER_DEBUG_TAG("NativeSession", 15) << "Session created" << IBRCOMMON_LOGGER_ENDL;
		}

		NativeSession::~NativeSession()
		{
			// send stop signal to the receiver
			_receiver.stop();

			// wait here until the receiver has been stopped
			_receiver.join();

			IBRCOMMON_LOGGER_DEBUG_TAG("NativeSession", 15) << "Session destroyed" << IBRCOMMON_LOGGER_ENDL;
		}

		const dtn::data::EID& NativeSession::getNodeEID() const
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
			const dtn::data::EID new_endpoint = dtn::core::BundleCore::local + "/" + suffix;

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
		}

		void NativeSession::resetEndpoint() throw ()
		{
			_endpoint = _registration.getDefaultEID();
		}

		void NativeSession::addEndpoint(const std::string &suffix) throw (NativeSessionException)
		{
			const dtn::data::EID new_endpoint = dtn::core::BundleCore::local + "/" + suffix;

			// error checking
			if (new_endpoint == dtn::data::EID())
			{
				throw NativeSessionException("given endpoint is not acceptable");
			}
			else
			{
				_registration.subscribe(new_endpoint);
			}
		}

		void NativeSession::removeEndpoint(const std::string &suffix) throw (NativeSessionException)
		{
			const dtn::data::EID old_endpoint = dtn::core::BundleCore::local + "/" + suffix;

			// error checking
			if (old_endpoint == dtn::data::EID())
			{
				throw NativeSessionException("given endpoint is not acceptable");
			}
			else
			{
				_registration.unsubscribe(old_endpoint);
			}
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
		}

		void NativeSession::next(RegisterIndex ri) throw (NativeSessionException)
		{
			try {
				const dtn::data::BundleID id = _bundle_queue.getnpop();
				load(ri, id);
			} catch (const ibrcommon::QueueUnblockedException&) {
				throw BundleNotFoundException();
			}
		}

		void NativeSession::load(RegisterIndex ri, const dtn::data::BundleID &id) throw (NativeSessionException)
		{
			// load the bundle
			try {
				_bundle[ri] = dtn::core::BundleCore::getInstance().getStorage().get(id);

				// process the bundle block (security, compression, ...)
				dtn::core::BundleCore::processBlocks(_bundle[ri]);
			} catch (const ibrcommon::Exception&) {
				throw BundleNotFoundException();
			}
		}

		const dtn::data::Bundle& NativeSession::get(RegisterIndex ri) const throw ()
		{
			return _bundle[ri];
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
		}

		void NativeSession::put(RegisterIndex ri, const dtn::data::Bundle &b) throw ()
		{
			// Copy the given bundle into the local register
			_bundle[ri] = b;
		}

		void NativeSession::write(RegisterIndex ri, const char *buf, const size_t len, const size_t offset)
		{
			try {
				dtn::data::PayloadBlock &payload = _bundle[ri].getBlock<dtn::data::PayloadBlock>();

				ibrcommon::BLOB::Reference ref = payload.getBLOB();
				ibrcommon::BLOB::iostream stream = ref.iostream();

				(*stream).seekp(offset);
				(*stream).write(buf, len);
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
				dtn::data::PayloadBlock &payload = _bundle[ri].push_back<dtn::data::PayloadBlock>();

				ibrcommon::BLOB::Reference ref = payload.getBLOB();
				ibrcommon::BLOB::iostream stream = ref.iostream();

				(*stream).seekp(offset);
				(*stream).write(buf, len);
			}
		}

		void NativeSession::read(RegisterIndex ri, char *buf, size_t &len, const size_t offset)
		{
			try {
				dtn::data::PayloadBlock &payload = _bundle[ri].getBlock<dtn::data::PayloadBlock>();

				ibrcommon::BLOB::Reference ref = payload.getBLOB();
				ibrcommon::BLOB::iostream stream = ref.iostream();

				(*stream).seekg(offset);
				(*stream).read(buf, len);

				len = (*stream).gcount();
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
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
			Registration &reg = _session._registration;
			try{
				while (_shutdown) {
					try {
						const dtn::data::MetaBundle id = reg.receiveMetaBundle();

						if (id.procflags & dtn::data::PrimaryBlock::APPDATA_IS_ADMRECORD) {
							// transform custody signals & status reports into notifies
							fireNotificationAdministrativeRecord(id);

							// announce the delivery of this bundle
							reg.delivered(id);
						} else {
							// notify the client about the new bundle
							_session.fireNotificationBundle(id);
						}
					} catch (const dtn::storage::NoBundleFoundException&) {
						reg.wait_for_bundle();
					}

					yield();
				}
			} catch (const ibrcommon::QueueUnblockedException &ex) {
				IBRCOMMON_LOGGER_DEBUG_TAG("NativeSession::BundleReceiver", 40) << ex.what() << IBRCOMMON_LOGGER_ENDL;
			} catch (const std::exception &ex) {
				IBRCOMMON_LOGGER_DEBUG_TAG("NativeSession::BundleReceiver", 10) << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void NativeSession::BundleReceiver::fireNotificationAdministrativeRecord(const dtn::data::MetaBundle &bundle)
		{
			// load the whole bundle
			const dtn::data::Bundle b = dtn::core::BundleCore::getInstance().getStorage().get(bundle);

			// get the payload block of the bundle
			const dtn::data::PayloadBlock &payload = b.getBlock<dtn::data::PayloadBlock>();

			try {
				// try to decode as status report
				dtn::data::StatusReportBlock report;
				report.read(payload);

				// fire the status report notification
				_session.fireNotificationStatusReport(report);
			} catch (const dtn::data::StatusReportBlock::WrongRecordException&) {
				// this is not a status report
			}

			try {
				// try to decode as custody signal
				dtn::data::CustodySignalBlock custody;
				custody.read(payload);

				// fire the custody signal notification
				_session.fireNotificationCustodySignal(custody);
			} catch (const dtn::data::CustodySignalBlock::WrongRecordException&) {
				// this is not a custody report
			}
		}
	} /* namespace net */
} /* namespace dtn */
