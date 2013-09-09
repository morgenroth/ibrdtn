/*
 * StreamBlock.cpp
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

#include "ibrdtn/data/StreamBlock.h"

namespace dtn
{
	namespace data
	{
		const dtn::data::block_t StreamBlock::BLOCK_TYPE = 242;

		dtn::data::Block* StreamBlock::Factory::create()
		{
			return new StreamBlock();
		}

		StreamBlock::StreamBlock()
		: dtn::data::Block(StreamBlock::BLOCK_TYPE), _seq(0)
		{
		}

		StreamBlock::~StreamBlock()
		{

		}

		Length StreamBlock::getLength() const
		{
			return _streamflags.getLength() + _seq.getLength();
		}

		std::ostream& StreamBlock::serialize(std::ostream &stream, Length&) const
		{
			stream << _streamflags;
			stream << _seq;
			return stream;
		}

		std::istream& StreamBlock::deserialize(std::istream &stream, const Length&)
		{
			stream >> _streamflags;
			stream >> _seq;
			return stream;
		}

		void StreamBlock::set(STREAM_FLAGS flag, const bool &value)
		{
			_streamflags.setBit(flag, value);
		}

		bool StreamBlock::get(STREAM_FLAGS flag) const
		{
			return _streamflags.getBit(flag);
		}

		void StreamBlock::setSequenceNumber(Number seq)
		{
			_seq = seq;
		}

		const Number& StreamBlock::getSequenceNumber() const
		{
			return _seq;
		}
	} /* namespace data */
} /* namespace dtn */
