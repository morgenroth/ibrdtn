/*
 * PlainSerializer.h
 *
 *   Copyright 2011 Johannes Morgenroth
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef PLAINSERIALIZER_H_
#define PLAINSERIALIZER_H_

#include <ibrdtn/data/Serializer.h>
#include <ibrdtn/data/Bundle.h>
#include <ibrdtn/data/PrimaryBlock.h>
#include <ibrdtn/data/Block.h>

namespace dtn
{
	namespace api
	{
		class PlainSerializer : public dtn::data::Serializer
		{
		public:
			PlainSerializer(std::ostream &stream, bool skip_payload = false);
			virtual ~PlainSerializer();

			dtn::data::Serializer &operator<<(const dtn::data::Bundle &obj);
			dtn::data::Serializer &operator<<(const dtn::data::PrimaryBlock &obj);
			dtn::data::Serializer &operator<<(const dtn::data::Block &obj);

			dtn::data::Serializer &serialize(ibrcommon::BLOB::iostream &obj, size_t limit = 0);

			size_t getLength(const dtn::data::Bundle &obj);
			size_t getLength(const dtn::data::PrimaryBlock &obj) const;
			size_t getLength(const dtn::data::Block &obj) const;

		private:
			std::ostream &_stream;
			bool _skip_payload;
		};

		class PlainDeserializer : public dtn::data::Deserializer
		{
		public:
			class BlockInserter;
			class UnknownBlockException;
			class BlockNotProcessableException;

			PlainDeserializer(std::istream &stream);
			virtual ~PlainDeserializer();

			dtn::data::Deserializer &operator>>(dtn::data::Bundle &obj);
			dtn::data::Deserializer &operator>>(dtn::data::PrimaryBlock &obj);
			dtn::data::Deserializer &operator>>(dtn::data::Block &obj);
			dtn::data::Deserializer &operator>>(ibrcommon::BLOB::iostream &obj);

			/**
			 * read a block from _stream and add it to a bundle using a given BlockInserter
			 * @param inserter the inserter that is used to insert the block into the bundle
			 * @return the block that was inserted
			 */
			dtn::data::Block& readBlock(BlockInserter inserter, bool payload_is_adm);

		private:
			/**
			 * deserialize a base64 encoded data stream into another stream
			 */
			dtn::data::Deserializer &operator>>(std::ostream &stream);

			std::istream &_stream;

		public:

			class BlockInserter
			{
			public:
				enum POSITION
				{
					FRONT,
					MIDDLE,
					END
				};

				BlockInserter(dtn::data::Bundle &bundle, POSITION alignment, int pos = 0);

				template <class T>
				T& insert();
				POSITION getAlignment() const;

				dtn::data::Block &insert(dtn::data::ExtensionBlock::Factory &f);

			private:
				dtn::data::Bundle *_bundle;
				POSITION _alignment;
				int _pos;
			};

			class PlainDeserializerException : public ibrcommon::Exception
			{
				public:
					PlainDeserializerException(string what = "") throw() : Exception(what)
					{
					}
			};

			class UnknownBlockException : public PlainDeserializerException
			{
				public:
					UnknownBlockException(string what = "unknown block") throw() : PlainDeserializerException(what)
					{
					}
			};

			class BlockNotProcessableException : public PlainDeserializerException
			{
				public:
					BlockNotProcessableException(string what = "block not processable") throw() : PlainDeserializerException(what)
					{
					}
			};

		};

		template <class T>
		T& PlainDeserializer::BlockInserter::insert()
		{
			switch (_alignment)
			{
			case FRONT:
				return _bundle->push_front<T>();
			case END:
				return _bundle->push_back<T>();
			default:
				if(_pos <= 0)
					return _bundle->push_front<T>();

				try
				{
					dtn::data::Block &prev_block = _bundle->getBlock(_pos-1);
					return _bundle->insert<T>(prev_block);
				}
				catch (const std::exception &ex)
				{
					return _bundle->push_back<T>();
				}
			}
		}
	}
}

#endif /* PLAINSERIALIZER_H_ */
