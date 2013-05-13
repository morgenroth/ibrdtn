/*
 * TrackingBlock.h
 *
 *  Created on: 15.01.2013
 *      Author: morgenro
 */

#ifndef TRACKINGBLOCK_H_
#define TRACKINGBLOCK_H_

#include <ibrdtn/data/Block.h>
#include <ibrdtn/data/Number.h>
#include <ibrdtn/data/DTNTime.h>
#include <ibrdtn/data/ExtensionBlock.h>

namespace dtn
{
	namespace data
	{
		class TrackingBlock : public dtn::data::Block
		{
		public:
			class Factory : public dtn::data::ExtensionBlock::Factory
			{
			public:
				Factory() : dtn::data::ExtensionBlock::Factory(TrackingBlock::BLOCK_TYPE) {};
				virtual ~Factory() {};
				virtual dtn::data::Block* create();
			};

			static const dtn::data::block_t BLOCK_TYPE;

			TrackingBlock();
			virtual ~TrackingBlock();

			virtual Length getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, Length &length) const;
			virtual std::istream &deserialize(std::istream &stream, const Length &length);

			virtual std::ostream &serialize_strict(std::ostream &stream, Length &length) const;
			virtual Length getLength_strict() const;

			class TrackingEntry
			{
			public:
				enum Flags
				{
					TIMESTAMP_PRESENT = 1,
					GEODATA_PRESENT = 2
				};

				TrackingEntry();
				TrackingEntry(const dtn::data::EID &eid);
				~TrackingEntry();

				bool getFlag(Flags f) const;
				void setFlag(Flags f, bool value);

				Bitset<Flags> flags;
				dtn::data::EID endpoint;
				dtn::data::DTNTime timestamp;

				friend std::ostream& operator<<(std::ostream &stream, const TrackingEntry &entry);
				friend std::istream& operator>>(std::istream &stream, TrackingEntry &entry);

				Length getLength() const;
			};

			typedef std::list<TrackingEntry> tracking_list;

			const tracking_list& getTrack() const;

			void append(const dtn::data::EID &eid);

		private:
			tracking_list _entries;
		};

		/**
		 * This creates a static block factory
		 */
		static TrackingBlock::Factory __TrackingBlockFactory__;
	} /* namespace data */
} /* namespace dtn */
#endif /* TRACKINGBLOCK_H_ */
