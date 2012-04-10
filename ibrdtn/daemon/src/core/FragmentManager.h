/*
 * FragmentManager.h
 *
 *  Created on: 03.02.2012
 *      Author: morgenro
 */

#ifndef FRAGMENTMANAGER_H_
#define FRAGMENTMANAGER_H_

#include "Component.h"
#include "core/EventReceiver.h"
#include <ibrdtn/data/MetaBundle.h>
#include <ibrdtn/data/BundleList.h>
#include <ibrcommon/thread/Queue.h>
#include <ibrcommon/thread/Mutex.h>
#include <list>
#include <set>

namespace dtn
{
	namespace core
	{
		class FragmentManager : public dtn::daemon::IndependentComponent, public dtn::core::EventReceiver
		{
		public:
			FragmentManager();
			virtual ~FragmentManager();

			void signal(const dtn::data::MetaBundle &meta);

			void __cancellation();

			void componentUp();
			void componentRun();
			void componentDown();

			void raiseEvent(const Event *evt);

			const std::string getName() const;

			/**
			 * Updates the offset of a transmission
			 * @param peer
			 * @param id
			 * @param offset
			 */
			static void setOffset(const dtn::data::EID &peer, const dtn::data::BundleID &id, size_t abs_offset);

			/**
			 * Get the offset of a transmission
			 * @param peer
			 * @param id
			 * @return
			 */
			static size_t getOffset(const dtn::data::EID &peer, const dtn::data::BundleID &id);

		private:
			class Transmission
			{
			public:
				Transmission();
				virtual ~Transmission();

				bool operator<(const Transmission &other) const;
				bool operator==(const Transmission &other) const;

				dtn::data::EID peer;
				dtn::data::BundleID id;
				size_t offset;
				size_t expires;
			};

			static void expire_offsets(size_t timestamp);
			static size_t get_payload_offset(const dtn::data::Bundle &bundle, size_t abs_offset);

			// search for similar bundles in the storage
			const std::list<dtn::data::MetaBundle> search(const dtn::data::MetaBundle &meta);

			ibrcommon::Queue<dtn::data::MetaBundle> _incoming;
			dtn::data::BundleList _fragments;
			bool _running;

			static ibrcommon::Mutex _offsets_mutex;
			static std::set<Transmission> _offsets;
		};
	} /* namespace core */
} /* namespace dtn */
#endif /* FRAGMENTMANAGER_H_ */
