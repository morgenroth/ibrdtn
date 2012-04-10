/*
 * RetransmissionExtension.h
 *
 *  Created on: 09.03.2010
 *      Author: morgenro
 */

#ifndef RETRANSMISSIONEXTENSION_H_
#define RETRANSMISSIONEXTENSION_H_

#include "routing/BaseRouter.h"
#include "ibrdtn/data/BundleID.h"
#include "ibrdtn/data/EID.h"
#include <queue>
#include <set>

namespace dtn
{
	namespace routing
	{
		class RetransmissionExtension : public BaseRouter::Extension
		{
		public:
			RetransmissionExtension();
			virtual ~RetransmissionExtension();

			void notify(const dtn::core::Event *evt);

		private:
			class RetransmissionData : public dtn::data::BundleID
			{
			public:
				RetransmissionData(const dtn::data::BundleID &id, dtn::data::EID destination, size_t retry = 2);
				virtual ~RetransmissionData();


				const dtn::data::EID destination;

				size_t getTimestamp() const;
				size_t getCount() const;

				RetransmissionData& operator++();
				RetransmissionData& operator++(int);

				bool operator!=(const RetransmissionData &obj);
				bool operator==(const RetransmissionData &obj);

			private:
				size_t _timestamp;
				size_t _count;
				const size_t retry;
			};

			std::queue<RetransmissionData> _queue;
			std::set<RetransmissionData> _set;
		};
	}
}

#endif /* RETRANSMISSIONEXTENSION_H_ */
