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

#include "streaming/StreamBundle.h"
#include <ibrdtn/api/Client.h>
#include <ibrdtn/data/Number.h>
#include <ibrdtn/data/Bundle.h>
#include <iostream>
#include <set>
#include <vector>

#ifndef BUNDLESTREAMBUF_H_
#define BUNDLESTREAMBUF_H_

class BundleStreamBuf : public std::basic_streambuf<char, std::char_traits<char> >
{
public:
	BundleStreamBuf(dtn::api::Client &client, StreamBundle &chunk, size_t min_buffer, size_t max_buffer, bool wait_seq_zero = false);
	virtual ~BundleStreamBuf();

	/**
	 * process incoming bundles
	 */
	void received(const dtn::data::Bundle &b);

	/**
	 * Flush the buffer and send out the buffered bundle
	 */
	void flush();

	/**
	 * Request Acks for the chunks
	 */
	void setRequestAck(bool val);

	/**
	 * Set the timeout for receiving bundles
	 */
	void setReceiveTimeout(dtn::data::Timeout timeout);

protected:
	virtual int sync();
	virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());
	virtual std::char_traits<char>::int_type underflow();
	std::char_traits<char>::int_type __underflow();

	void __flush();

	size_t __get_next_buffer_size() const;

private:
	class Chunk
	{
	public:
		Chunk(const dtn::data::Bundle &b);
		virtual ~Chunk();

		bool operator==(const Chunk& other) const;
		bool operator<(const Chunk& other) const;

		dtn::data::Bundle _bundle;
		dtn::data::Number _seq;
	};

	// Input buffer
	std::vector<char> _in_buf;
	// Output buffer
	std::vector<char> _out_buf;

	dtn::api::Client &_client;
	StreamBundle &_chunk;

	// minimum buffer size
	size_t _min_buf_size;

	// maximum buffer size
	size_t _max_buf_size;

	ibrcommon::Conditional _chunks_cond;
	std::set<Chunk> _chunks;
	size_t _chunk_offset;

	dtn::data::Number _in_seq;
	bool _streaming;
	bool _request_ack;
	bool _flush_request;

	dtn::data::Timeout _receive_timeout;
};

#endif /* BUNDLESTREAMBUF_H_ */
