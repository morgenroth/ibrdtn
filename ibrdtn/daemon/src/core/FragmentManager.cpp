/*
 * FragmentManager.cpp
 *
 *  Created on: 03.02.2012
 *      Author: morgenro
 */

#include "core/FragmentManager.h"
#include "core/BundleCore.h"
#include "core/EventSwitch.h"
#include "core/TimeEvent.h"
#include "core/BundlePurgeEvent.h"
#include "net/BundleReceivedEvent.h"
#include "routing/QueueBundleEvent.h"
#include <ibrdtn/data/BundleMerger.h>
#include <ibrdtn/utils/Clock.h>
#include <ibrcommon/Logger.h>
#include <ibrcommon/thread/MutexLock.h>

namespace dtn
{
	namespace core
	{
		ibrcommon::Mutex FragmentManager::_offsets_mutex;
		std::set<FragmentManager::Transmission> FragmentManager::_offsets;

		FragmentManager::FragmentManager()
		 : _running(false)
		{
		}

		FragmentManager::~FragmentManager()
		{
		}

		const std::string FragmentManager::getName() const
		{
			return "FragmentManager";
		}

		void FragmentManager::__cancellation()
		{
			_running = false;
			_incoming.abort();
		}

		void FragmentManager::componentUp()
		{
			bindEvent(dtn::routing::QueueBundleEvent::className);
			bindEvent(dtn::core::TimeEvent::className);
			_running = true;
		}

		void FragmentManager::componentRun()
		{
			// TODO: scan storage for fragments to reassemble on startup

			// create a task loop to reassemble fragments asynchronously
			try {
				while (_running)
				{
					dtn::data::MetaBundle meta = _incoming.getnpop(true);

					// search for matching bundles
					const std::list<dtn::data::MetaBundle> list = search(meta);

					IBRCOMMON_LOGGER_DEBUG(20) << "found " << list.size() << " fragments similar to bundle " << meta.toString() << IBRCOMMON_LOGGER_ENDL;

					// TODO: drop fragments if other fragments available containing the same payload

					// check first if all fragment are available
					std::set<BundleMerger::Chunk> chunks;
					for (std::list<dtn::data::MetaBundle>::const_iterator iter = list.begin(); iter != list.end(); iter++)
					{
						const dtn::data::MetaBundle &m = (*iter);
						if (meta.payloadlength > 0)
						{
							BundleMerger::Chunk chunk(m.offset, m.payloadlength);
							chunks.insert(chunk);
						}
					}

					// wait for the next bundle if the fragment is not complete
					if (!BundleMerger::Chunk::isComplete(meta.appdatalength, chunks)) continue;

					// create a new bundle merger container
					dtn::data::BundleMerger::Container c = dtn::data::BundleMerger::getContainer();

					for (std::list<dtn::data::MetaBundle>::const_iterator iter = list.begin(); iter != list.end(); iter++)
					{
						const dtn::data::MetaBundle &meta = (*iter);

						if (meta.payloadlength > 0)
						{
							IBRCOMMON_LOGGER_DEBUG(20) << "fragment: " << (*iter).toString() << IBRCOMMON_LOGGER_ENDL;

							try {
								// load bundle from storage
								dtn::data::Bundle bundle = dtn::core::BundleCore::getInstance().getStorage().get(*iter);

								// merge the bundle
								c << bundle;
							} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) {
								IBRCOMMON_LOGGER(error) << "could not load fragment to merge bundle" << IBRCOMMON_LOGGER_ENDL;
							};
						}
					}

					if (c.isComplete())
					{
						dtn::data::Bundle &merged = c.getBundle();

						// raise default bundle received event
						dtn::net::BundleReceivedEvent::raise(dtn::core::BundleCore::local, merged, true, true);

						// delete all fragments of the merged bundle
						for (std::list<dtn::data::MetaBundle>::const_iterator iter = list.begin(); iter != list.end(); iter++)
						{
							dtn::core::BundlePurgeEvent::raise(*iter);
						}
					}
				}
			} catch (const ibrcommon::QueueUnblockedException&) { }
		}

		void FragmentManager::componentDown()
		{
			unbindEvent(dtn::routing::QueueBundleEvent::className);
			unbindEvent(dtn::core::TimeEvent::className);
		}

		void FragmentManager::raiseEvent(const Event *evt)
		{
			try {
				const dtn::core::TimeEvent &time = dynamic_cast<const dtn::core::TimeEvent&>(*evt);
				_fragments.expire(time.getTimestamp());
				FragmentManager::expire_offsets(time.getTimestamp());
			} catch (const std::bad_cast&) {}

			try {
				const dtn::routing::QueueBundleEvent &queued = dynamic_cast<const dtn::routing::QueueBundleEvent&>(*evt);

				// process fragments
				if (queued.bundle.fragment) signal(queued.bundle);
			} catch (const std::bad_cast&) {}
		}

		void FragmentManager::signal(const dtn::data::MetaBundle &meta)
		{
			// do not merge a bundle if it is non-local and singleton
			// we only touch local and group bundles which might be delivered locally
			if (meta.get(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON))
			{
				if (meta.destination.getNode() != dtn::core::BundleCore::local)
				{
					return;
				}
			}

			// push the meta bundle into the incoming queue
			_incoming.push(meta);
		}

		const std::list<dtn::data::MetaBundle> FragmentManager::search(const dtn::data::MetaBundle &meta)
		{
			class BundleFilter : public dtn::storage::BundleStorage::BundleFilterCallback
			{
			public:
				BundleFilter(const dtn::data::MetaBundle &meta)
				 : _similar(meta)
				{};

				virtual ~BundleFilter() {};

				virtual size_t limit() const { return 0; };

				virtual bool shouldAdd(const dtn::data::MetaBundle &meta) const
				{
					// fragments only
					if (!meta.fragment) return false;

					// with the same unique bundle id
					if (meta.source != _similar.source) return false;
					if (meta.timestamp != _similar.timestamp) return false;
					if (meta.sequencenumber != _similar.sequencenumber) return false;

					return true;
				};

			private:
				const dtn::data::MetaBundle &_similar;
			};

			// create a bundle filter
			BundleFilter filter(meta);
			dtn::storage::BundleStorage &storage = dtn::core::BundleCore::getInstance().getStorage();
			return storage.get(filter);
		}

		void FragmentManager::setOffset(const dtn::data::EID &peer, const dtn::data::BundleID &id, size_t abs_offset)
		{
			try {
				Transmission t;
				dtn::data::Bundle b = dtn::core::BundleCore::getInstance().getStorage().get(id);
				t.offset = get_payload_offset(b, abs_offset);

				if (t.offset <= 0) return;

				t.id = id;
				t.peer = peer;
				t.expires = dtn::utils::Clock::getExpireTime( b );

				IBRCOMMON_LOGGER_DEBUG(20) << "[FragmentManager] store offset of partial transmitted bundle " <<
						id.toString() << "; offset: " << t.offset << " (" << abs_offset << ")" << IBRCOMMON_LOGGER_ENDL;

				ibrcommon::MutexLock l(_offsets_mutex);
				_offsets.erase(t);
				_offsets.insert(t);
			} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) { };
		}

		size_t FragmentManager::getOffset(const dtn::data::EID &peer, const dtn::data::BundleID &id)
		{
			ibrcommon::MutexLock l(_offsets_mutex);
			for (std::set<Transmission>::const_iterator iter = _offsets.begin(); iter != _offsets.end(); iter++)
			{
				const Transmission &t = (*iter);
				if (t.peer != peer) continue;
				if (t.id != id) continue;
				return t.offset;
			}

			return 0;
		}

		size_t FragmentManager::get_payload_offset(const dtn::data::Bundle &bundle, size_t abs_offset)
		{
			dtn::data::DefaultSerializer serializer(std::cout);
			size_t header = serializer.getLength((dtn::data::PrimaryBlock&)bundle);

			const std::list<const dtn::data::Block*> blocks = bundle.getBlocks();
			for (std::list<const dtn::data::Block*>::const_iterator iter = blocks.begin(); iter != blocks.end(); iter++)
			{
				const dtn::data::Block *b = (*iter);
				header += serializer.getLength(*b);

				try {
					const dtn::data::PayloadBlock &payload = dynamic_cast<const dtn::data::PayloadBlock&>(*b);
					header -= payload.getLength();
					if (abs_offset < header) return 0;
					return abs_offset - header;
				} catch (std::bad_cast&) { };
			}

			return 0;
		}

		void FragmentManager::expire_offsets(size_t timestamp)
		{
			ibrcommon::MutexLock l(_offsets_mutex);
			for (std::set<Transmission>::iterator iter = _offsets.begin(); iter != _offsets.end();)
			{
				const Transmission &t = (*iter);
				if (t.expires >= timestamp) return;
				_offsets.erase(iter++);
			}
		}

		FragmentManager::Transmission::Transmission()
		 : offset(0), expires(0)
		{
		}

		FragmentManager::Transmission::~Transmission()
		{
		}

		bool FragmentManager::Transmission::operator<(const Transmission &other) const
		{
			if (expires < other.expires) return true;
			if (expires != other.expires) return false;

			if (peer < other.peer) return true;
			if (peer != other.peer) return false;

			return (id < other.id);
		}

		bool FragmentManager::Transmission::operator==(const Transmission &other) const
		{
			if (expires != other.expires) return false;
			if (peer != other.peer) return false;
			if (id != other.id) return false;

			return true;
		}
	} /* namespace core */
} /* namespace dtn */
