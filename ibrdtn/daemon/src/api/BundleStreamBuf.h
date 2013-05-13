/*
 * BundleStreamBuf.h
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

#ifndef BUNDLESTREAMBUF_H_
#define BUNDLESTREAMBUF_H_

#include <ibrdtn/data/MetaBundle.h>
#include <ibrcommon/thread/Conditional.h>
#include <iostream>

namespace dtn
{
	namespace api
	{
		class BundleStreamBufCallback
		{
		public:
			virtual ~BundleStreamBufCallback() { };
			virtual void put(dtn::data::Bundle &b) = 0;
			virtual dtn::data::MetaBundle get(const dtn::data::Timeout timeout = 0) = 0;
			virtual void delivered(const dtn::data::MetaBundle &b) = 0;
		};

		class BundleStreamBuf : public std::basic_streambuf<char, std::char_traits<char> >
		{
		public:
			// The size of the input and output buffers.
			static const dtn::data::Length BUFF_SIZE = 5120;

			BundleStreamBuf(BundleStreamBufCallback &callback, const dtn::data::Length chunk_size = 4096, bool wait_seq_zero = false);
			virtual ~BundleStreamBuf();

			void setChunkSize(const dtn::data::Length &size);
			void setTimeout(const dtn::data::Timeout &timeout);

		protected:
			virtual int sync();
			virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());
			virtual std::char_traits<char>::int_type underflow();

		private:
			class Chunk
			{
			public:
				Chunk(const dtn::data::MetaBundle &m);
				virtual ~Chunk();

				bool operator==(const Chunk& other) const;
				bool operator<(const Chunk& other) const;

				void load();

				dtn::data::MetaBundle _meta;
				dtn::data::Number _seq;
				bool _first;
				bool _last;
			};

			void flushPayload(bool final = false);

			static void append(ibrcommon::BLOB::Reference &ref, const char* data, const dtn::data::Length &length);

			BundleStreamBufCallback &_callback;

			// Input buffer
			std::vector<char> _in_buf;
			// Output buffer
			std::vector<char> _out_buf;

			dtn::data::Length _chunk_size;

			ibrcommon::BLOB::Reference _chunk_payload;

			std::set<Chunk> _chunks;
			dtn::data::Length _chunk_offset;

			dtn::data::Bundle _current_bundle;

			dtn::data::Number _in_seq;
			dtn::data::Number _out_seq;

			bool _streaming;

			// is set to true, when the first bundle is still not sent
			bool _first_chunk;

			// if true, the last chunk was received before
			bool _last_chunk_received;

			dtn::data::Timeout _timeout_receive;
		};
	} /* namespace data */
} /* namespace dtn */
#endif /* BUNDLESTREAMBUF_H_ */
