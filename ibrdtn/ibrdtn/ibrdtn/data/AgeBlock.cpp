/*
 * AgeBlock.cpp
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

#include "ibrdtn/data/AgeBlock.h"
#include <ibrcommon/TimeMeasurement.h>

namespace dtn
{
	namespace data
	{
		const dtn::data::block_t AgeBlock::BLOCK_TYPE = 10;

		dtn::data::Block* AgeBlock::Factory::create()
		{
			return new AgeBlock();
		}

		AgeBlock::AgeBlock()
		 : dtn::data::Block(AgeBlock::BLOCK_TYPE)
		{
			_time.start();

			// set the replicate in every fragment bit
			set(REPLICATE_IN_EVERY_FRAGMENT, true);
		}

		AgeBlock::~AgeBlock()
		{
		}

		dtn::data::Number AgeBlock::getMicroseconds() const
		{
			ibrcommon::TimeMeasurement time = this->_time;
			time.stop();
			return _age + static_cast<size_t>(time.getMicroseconds());
		}

		dtn::data::Number AgeBlock::getSeconds() const
		{
			return getMicroseconds() / 1000000;
		}

		void AgeBlock::addMicroseconds(const dtn::data::Number &value)
		{
			_age += value;
		}

		void AgeBlock::setMicroseconds(const dtn::data::Number &value)
		{
			_age = value;
		}

		void AgeBlock::addSeconds(const dtn::data::Number &value)
		{
			_age += (value * 1000000);
		}

		void AgeBlock::setSeconds(const dtn::data::Number &value)
		{
			_age = value * 1000000;
		}

		dtn::data::Length AgeBlock::getLength() const
		{
			return getMicroseconds().getLength();
		}

		std::ostream& AgeBlock::serialize(std::ostream &stream, dtn::data::Length &length) const
		{
			const dtn::data::Number value = getMicroseconds();
			stream << value;
			length += value.getLength();
			return stream;
		}

		std::istream& AgeBlock::deserialize(std::istream &stream, const dtn::data::Length&)
		{
			stream >> _age;
			_time.start();
			return stream;
		}

		dtn::data::Length AgeBlock::getLength_strict() const
		{
			return 1;
		}

		std::ostream& AgeBlock::serialize_strict(std::ostream &stream, dtn::data::Length &length) const
		{
			// we have to ignore the age field, because this is very dynamic data
			stream << (char)0;
			length += 1;
			return stream;
		}
	}
}
