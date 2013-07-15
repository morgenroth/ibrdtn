/*
 * Registration.cpp
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
#include "Configuration.h"
#include "api/Registration.h"
#include "storage/BundleStorage.h"
#include "core/BundleCore.h"
#include "core/BundleEvent.h"
#include "core/BundlePurgeEvent.h"
#include "core/FragmentManager.h"
#include "net/BundleReceivedEvent.h"

#ifdef HAVE_SQLITE
#include "storage/SQLiteBundleStorage.h"
#endif

#ifdef WITH_COMPRESSION
#include <ibrdtn/data/CompressedPayloadBlock.h>
#endif

#ifdef WITH_BUNDLE_SECURITY
#include "security/SecurityManager.h"
#endif

#include <ibrdtn/data/TrackingBlock.h>
#include <ibrdtn/data/AgeBlock.h>

#include <ibrdtn/utils/Clock.h>
#include <ibrdtn/utils/Random.h>
#include <ibrcommon/Logger.h>

#include <limits.h>
#include <stdint.h>

namespace dtn
{
	namespace api
	{
		const std::string Registration::TAG = "Registration";
		std::set<std::string> Registration::_handles;

		const std::string Registration::alloc_handle()
		{
			static dtn::utils::Random rand;

			std::string new_handle = rand.gen_chars(16);

			// if the local host is configured with an IPN address
			if (dtn::core::BundleCore::local.isCompressable())
			{
				// .. then use 32-bit numbers only
				uint32_t *int_handle = (uint32_t*)new_handle.c_str();
				std::stringstream ss;
				ss << *int_handle;
				new_handle = ss.str();
			}

			while (_handles.find(new_handle) != _handles.end())
			{
				new_handle = rand.gen_chars(16);

				// if the local host is configured with an IPN address
				if (dtn::core::BundleCore::local.isCompressable())
				{
					// .. then use 32-bit numbers only
					uint32_t *int_handle = (uint32_t*)new_handle.c_str();
					std::stringstream ss;
					ss << *int_handle;
					new_handle = ss.str();
				}
			}

			Registration::_handles.insert(new_handle);

			return new_handle;
		}

		void Registration::free_handle(const std::string &handle)
		{
			Registration::_handles.erase(handle);
		}

		Registration::Registration()
		 : _handle(alloc_handle()),
		   _default_eid(core::BundleCore::local.add( dtn::core::BundleCore::local.getDelimiter() + _handle) ),
		   _persistent(false), _detached(false), _expiry(0), _filter_fragments(true)
		{
		}

		Registration::~Registration()
		{
			free_handle(_handle);
		}

		void Registration::notify(const NOTIFY_CALL call)
		{
			ibrcommon::MutexLock l(_wait_for_cond);
			if (call == NOTIFY_BUNDLE_AVAILABLE)
			{
				_no_more_bundles = false;
				_wait_for_cond.signal(true);
			}
			else
			{
				_notify_queue.push(call);
			}
		}

		void Registration::wait_for_bundle(size_t timeout)
		{
			ibrcommon::MutexLock l(_wait_for_cond);

			while (_no_more_bundles)
			{
				if (timeout > 0)
				{
					_wait_for_cond.wait(timeout);
				}
				else
				{
					_wait_for_cond.wait();
				}
			}
		}

		Registration::NOTIFY_CALL Registration::wait()
		{
			return _notify_queue.getnpop(true);
		}

		bool Registration::hasSubscribed(const dtn::data::EID &endpoint)
		{
			ibrcommon::MutexLock l(_endpoints_lock);
			return (_endpoints.find(endpoint) != _endpoints.end());
		}

		const std::set<dtn::data::EID> Registration::getSubscriptions()
		{
			ibrcommon::MutexLock l(_endpoints_lock);
			return _endpoints;
		}

		void Registration::delivered(const dtn::data::MetaBundle &m) const
		{
			// raise bundle event
			dtn::core::BundleEvent::raise(m, dtn::core::BUNDLE_DELIVERED);

			if (m.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON))
			{
				dtn::core::BundlePurgeEvent::raise(m);
			}
		}

		dtn::data::Bundle Registration::receive() throw (dtn::storage::NoBundleFoundException)
		{
			// get the global storage
			dtn::storage::BundleStorage &storage = dtn::core::BundleCore::getInstance().getStorage();

			// get the next bundles as MetaBundle
			dtn::data::MetaBundle b = receiveMetaBundle();

			// load the bundle
			return storage.get(b);
		}

		dtn::data::MetaBundle Registration::receiveMetaBundle() throw (dtn::storage::NoBundleFoundException)
		{
			ibrcommon::MutexLock l(_receive_lock);

			while(true)
			{
				try {
					// get the first bundle in the queue
					dtn::data::MetaBundle b = _queue.pop();
					return b;
				} catch (const ibrcommon::QueueUnblockedException &e) {
					if (e.reason == ibrcommon::QueueUnblockedException::QUEUE_ABORT)
					{
						IBRCOMMON_LOGGER_DEBUG_TAG(Registration::TAG, 25) << "search for more bundles" << IBRCOMMON_LOGGER_ENDL;

						// query for new bundles
						underflow();
					}
				}
				catch(const dtn::storage::NoBundleFoundException & ){
				}
			}

			throw dtn::storage::NoBundleFoundException();
		}

		void Registration::underflow()
		{
			bool fragment_conf = dtn::daemon::Configuration::getInstance().getNetwork().doFragmentation();

			// expire outdated bundles in the list
			_queue.expire(dtn::utils::Clock::getTime());

			/**
			 * search for bundles in the storage
			 */
#ifdef HAVE_SQLITE
			class BundleFilter : public dtn::storage::BundleSelector, public dtn::storage::SQLiteDatabase::SQLBundleQuery
#else
			class BundleFilter : public dtn::storage::BundleSelector
#endif
			{
			public:
				BundleFilter(const std::set<dtn::data::EID> endpoints, const RegistrationQueue &queue, bool loopback, bool fragment_filter)
				 : _endpoints(endpoints), _queue(queue), _loopback(loopback), _fragment_filter(fragment_filter)
				{};

				virtual ~BundleFilter() {};

				virtual dtn::data::Size limit() const throw () { return dtn::core::BundleCore::max_bundles_in_transit; };

				virtual bool shouldAdd(const dtn::data::MetaBundle &meta) const throw (dtn::storage::BundleSelectorException)
				{
					// filter fragments if requested
					if (meta.fragment && _fragment_filter)
					{
						return false;
					}

					if (_endpoints.find(meta.destination) == _endpoints.end())
					{
						return false;
					}

					// filter own bundles
					if (!_loopback)
					{
						if (_endpoints.find(meta.source) != _endpoints.end())
						{
							return false;
						}
					}

					IBRCOMMON_LOGGER_DEBUG_TAG(Registration::TAG, 30) << "search bundle in the list of delivered bundles: " << meta.toString() << IBRCOMMON_LOGGER_ENDL;

					if (_queue.has(meta))
					{
						return false;
					}

					return true;
				};

#ifdef HAVE_SQLITE
				const std::string getWhere() const throw ()
				{
					if (_endpoints.size() > 1)
					{
						std::string where = "(";

						for (size_t i = _endpoints.size() - 1; i > 0; i--)
						{
							where += "destination = ? OR ";
						}

						return where + "destination = ?)";
					}
					else if (_endpoints.size() == 1)
					{
						return "destination = ?";
					}
					else
					{
						return "destination = null";
					}
				};

				int bind(sqlite3_stmt *st, int offset) const throw ()
				{
					int o = offset;

					for (std::set<dtn::data::EID>::const_iterator iter = _endpoints.begin(); iter != _endpoints.end(); ++iter)
					{
						const std::string data = (*iter).getString();

						sqlite3_bind_text(st, o, data.c_str(), static_cast<int>(data.size()), SQLITE_TRANSIENT);
						o++;
					}

					return o;
				}
#endif

			private:
				const std::set<dtn::data::EID> _endpoints;
				const RegistrationQueue &_queue;
				const bool _loopback;
				const bool _fragment_filter;
			} filter(_endpoints, _queue, false, fragment_conf && _filter_fragments);

			// query the database for more bundles
			ibrcommon::MutexLock l(_endpoints_lock);

			try {
				dtn::core::BundleCore::getInstance().getSeeker().get( filter, _queue );
			} catch (const dtn::storage::NoBundleFoundException&) {
				_no_more_bundles = true;
				throw;
			}
		}

		Registration::RegistrationQueue::RegistrationQueue()
		{
		}

		Registration::RegistrationQueue::~RegistrationQueue()
		{
		}

		void Registration::RegistrationQueue::put(const dtn::data::MetaBundle &bundle) throw ()
		{
			try {
				_queue.push(bundle);

				ibrcommon::MutexLock l(_lock);
				_recv_bundles.add(bundle);

				IBRCOMMON_LOGGER_DEBUG_TAG(Registration::TAG, 10) << "[RegistrationQueue] add bundle to list of delivered bundles: " << bundle.toString() << IBRCOMMON_LOGGER_ENDL;
			} catch (const ibrcommon::Exception&) { }
		}

		dtn::data::MetaBundle Registration::RegistrationQueue::pop() throw (const ibrcommon::QueueUnblockedException)
		{
			return _queue.getnpop(false);
		}

		bool Registration::RegistrationQueue::has(const dtn::data::BundleID &bundle) const throw ()
		{
			ibrcommon::MutexLock l(const_cast<ibrcommon::Mutex&>(_lock));
			return _recv_bundles.has(bundle);
		}

		void Registration::RegistrationQueue::expire(const dtn::data::Timestamp &timestamp) throw ()
		{
			ibrcommon::MutexLock l(_lock);
			_recv_bundles.expire(timestamp);
		}

		void Registration::RegistrationQueue::abort() throw ()
		{
			_queue.abort();
		}

		void Registration::RegistrationQueue::reset() throw ()
		{
			_queue.reset();
		}

		void Registration::subscribe(const dtn::data::EID &endpoint)
		{
			{
				ibrcommon::MutexLock l(_endpoints_lock);

				// add endpoint to the local set
				_endpoints.insert(endpoint);
			}

			// trigger the search for new bundles
			notify(NOTIFY_BUNDLE_AVAILABLE);
		}

		void Registration::unsubscribe(const dtn::data::EID &endpoint)
		{
			ibrcommon::MutexLock l(_endpoints_lock);
			_endpoints.erase(endpoint);
		}

		/**
		 * compares the local handle with the given one
		 */
		bool Registration::operator==(const std::string &other) const
		{
			return (_handle == other);
		}

		/**
		 * compares another registration with this one
		 */
		bool Registration::operator==(const Registration &other) const
		{
			return (_handle == other._handle);
		}

		/**
		 * compares and order a registration (using the handle)
		 */
		bool Registration::operator<(const Registration &other) const
		{
			return (_handle < other._handle);
		}

		void Registration::abort()
		{
			_queue.abort();
			_notify_queue.abort();

			ibrcommon::MutexLock l(_wait_for_cond);
			_wait_for_cond.abort();
		}

		const dtn::data::EID& Registration::getDefaultEID() const
		{
			return _default_eid;
		}

		const std::string& Registration::getHandle() const
		{
			return _handle;
		}

		void Registration::setPersistent(ibrcommon::Timer::time_t lifetime)
		{
			_expiry = lifetime + ibrcommon::Timer::get_current_time();
			_persistent = true;
		}

		void Registration::unsetPersistent()
		{
			_persistent = false;
		}

		bool Registration::isPersistent()
		{
			if(_expiry <= ibrcommon::Timer::get_current_time())
			{
				_persistent = false;
			}

			return _persistent;
		}

		bool Registration::isPersistent() const
		{
			if(_expiry <= ibrcommon::Timer::get_current_time())
			{
				return false;
			}

			return _persistent;
		}

		void Registration::setFilterFragments(bool val)
		{
			_filter_fragments = val;
		}

		ibrcommon::Timer::time_t Registration::getExpireTime() const
		{
			if(!isPersistent()) throw NotPersistentException("Registration is not persistent.");

			return _expiry;

		}

		void Registration::attach()
		{
			ibrcommon::MutexLock l(_attach_lock);
			if(!_detached) throw AlreadyAttachedException("Registration is already attached to a client.");

			_detached = false;
		}

		void Registration::detach()
		{
			ibrcommon::MutexLock l1(_wait_for_cond);
			ibrcommon::MutexLock l2(_attach_lock);

			_detached = true;

			_queue.reset();
			_notify_queue.reset();

			_wait_for_cond.reset();
		}

		void Registration::processIncomingBundle(const dtn::data::EID &source, dtn::data::Bundle &bundle)
		{
			// check address fields for "api:me", this has to be replaced
			static const dtn::data::EID clienteid("api:me");

			// set the source address to the sending EID
			bundle.source = source;

			if (bundle.destination == clienteid) bundle.destination = source;
			if (bundle.reportto == clienteid) bundle.reportto = source;
			if (bundle.custodian == clienteid) bundle.custodian = source;

			// if the timestamp is not set, add a ageblock
			if (bundle.timestamp == 0)
			{
				// check for ageblock
				try {
					bundle.find<dtn::data::AgeBlock>();
				} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) {
					// add a new ageblock
					bundle.push_front<dtn::data::AgeBlock>();
				}
			}

			// modify TrackingBlock
			try {
				dtn::data::TrackingBlock &track = bundle.find<dtn::data::TrackingBlock>();
				track.append(dtn::core::BundleCore::local);
			} catch (const dtn::data::Bundle::NoSuchBlockFoundException&) { };

#ifdef WITH_COMPRESSION
			// if the compression bit is set, then compress the bundle
			if (bundle.get(dtn::data::PrimaryBlock::IBRDTN_REQUEST_COMPRESSION))
			{
				try {
					dtn::data::CompressedPayloadBlock::compress(bundle, dtn::data::CompressedPayloadBlock::COMPRESSION_ZLIB);
				} catch (const ibrcommon::Exception &ex) {
					IBRCOMMON_LOGGER_TAG(Registration::TAG, warning) << "compression of bundle failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
				};
			}
#endif

#ifdef WITH_BUNDLE_SECURITY
			// if the encrypt bit is set, then try to encrypt the bundle
			if (bundle.get(dtn::data::PrimaryBlock::DTNSEC_REQUEST_ENCRYPT))
			{
				try {
					dtn::security::SecurityManager::getInstance().encrypt(bundle);

					bundle.set(dtn::data::PrimaryBlock::DTNSEC_REQUEST_ENCRYPT, false);
				} catch (const dtn::security::SecurityManager::KeyMissingException&) {
					// sign requested, but no key is available
					IBRCOMMON_LOGGER_TAG(Registration::TAG, warning) << "No key available for encrypt process." << IBRCOMMON_LOGGER_ENDL;
				} catch (const dtn::security::SecurityManager::EncryptException&) {
					IBRCOMMON_LOGGER_TAG(Registration::TAG, warning) << "Encryption of bundle failed." << IBRCOMMON_LOGGER_ENDL;
				}
			}

			// if the sign bit is set, then try to sign the bundle
			if (bundle.get(dtn::data::PrimaryBlock::DTNSEC_REQUEST_SIGN))
			{
				try {
					dtn::security::SecurityManager::getInstance().sign(bundle);

					bundle.set(dtn::data::PrimaryBlock::DTNSEC_REQUEST_SIGN, false);
				} catch (const dtn::security::SecurityManager::KeyMissingException&) {
					// sign requested, but no key is available
					IBRCOMMON_LOGGER_TAG(Registration::TAG, warning) << "No key available for sign process." << IBRCOMMON_LOGGER_ENDL;
				}
			}
#endif

			// get the payload size maximum
			size_t maxPayloadLength = dtn::daemon::Configuration::getInstance().getLimit("payload");

			// check if fragmentation is enabled
			// do not try pro-active fragmentation if the payload length is not limited
			if (dtn::daemon::Configuration::getInstance().getNetwork().doFragmentation() && (maxPayloadLength > 0))
			{
				try {
					std::list<dtn::data::Bundle> fragments;

					dtn::core::FragmentManager::split(bundle, maxPayloadLength, fragments);

					//for each fragment raise bundle received event
					for(std::list<dtn::data::Bundle>::iterator it = fragments.begin(); it != fragments.end(); ++it)
					{
						// raise default bundle received event
						dtn::net::BundleReceivedEvent::raise(source, *it, true);
					}

					return;
				} catch (const FragmentationProhibitedException&) {
				} catch (const FragmentationNotNecessaryException&) {
				} catch (const FragmentationAbortedException&) {
					// drop the bundle
					return;
				}
			}

			// raise default bundle received event
			dtn::net::BundleReceivedEvent::raise(source, bundle, true);
		}
	}
}
