/*
 * TrackingBlock.h
 *
 *  Created on: 15.01.2013
 *      Author: morgenro
 */

#ifndef TRACKINGBLOCK_H_
#define TRACKINGBLOCK_H_

#include <ibrdtn/data/Block.h>
#include <ibrdtn/data/SDNV.h>
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

			enum { BLOCK_TYPE = (dtn::data::block_t)193 };

			TrackingBlock();
			virtual ~TrackingBlock();

			virtual size_t getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, size_t &length) const;
			virtual std::istream &deserialize(std::istream &stream, const size_t length);

			virtual std::ostream &serialize_strict(std::ostream &stream, size_t &length) const;
			virtual size_t getLength_strict() const;

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

				uint64_t flags;
				dtn::data::EID endpoint;
				dtn::data::DTNTime timestamp;

				friend std::ostream& operator<<(std::ostream &stream, const TrackingEntry &entry);
				friend std::istream& operator>>(std::istream &stream, TrackingEntry &entry);

				size_t getLength() const;
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
