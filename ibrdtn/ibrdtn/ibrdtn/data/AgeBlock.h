/*
 * AgeBlock.h
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

#include <ibrdtn/data/Block.h>
#include <ibrdtn/data/SDNV.h>
#include <ibrdtn/data/ExtensionBlock.h>
#include <ibrcommon/TimeMeasurement.h>

#ifndef AGEBLOCK_H_
#define AGEBLOCK_H_

namespace dtn
{
	namespace data
	{
		class AgeBlock : public dtn::data::Block
		{
		public:
			class Factory : public dtn::data::ExtensionBlock::Factory
			{
			public:
				Factory() : dtn::data::ExtensionBlock::Factory(AgeBlock::BLOCK_TYPE) {};
				virtual ~Factory() {};
				virtual dtn::data::Block* create();
			};

			enum { BLOCK_TYPE = (dtn::data::block_t)10 };

			AgeBlock();
			virtual ~AgeBlock();

			virtual size_t getLength() const;
			virtual std::ostream &serialize(std::ostream &stream, size_t &length) const;
			virtual std::istream &deserialize(std::istream &stream, const size_t length);

			virtual std::ostream &serialize_strict(std::ostream &stream, size_t &length) const;
			virtual size_t getLength_strict() const;

			size_t getMicroseconds() const;
			size_t getSeconds() const;

			/**
			 * set the age
			 */
			void setSeconds(size_t value);

			/**
			 * add a value to the age
			 */
			void addSeconds(size_t value);

			/**
			 * set the age
			 */
			void setMicroseconds(size_t value);

			/**
			 * add microseconds to the ageblock
			 */
			void addMicroseconds(size_t value);

		private:
			dtn::data::SDNV _age;
			ibrcommon::TimeMeasurement _time;
		};

		/**
		 * This creates a static block factory
		 */
		static AgeBlock::Factory __AgeBlockFactory__;
	}
}

#endif /* AGEBLOCK_H_ */
