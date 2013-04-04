/*
 * BundleStream.h
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

#ifndef BUNDLESTREAM_H_
#define BUNDLESTREAM_H_

#include "streaming/StreamBundle.h"
#include "streaming/BundleStreamBuf.h"
#include <ibrdtn/api/Client.h>
#include <ibrdtn/data/EID.h>
#include <ibrdtn/data/SDNV.h>
#include <ibrdtn/data/StreamBlock.h>
#include <ibrdtn/data/BundleID.h>
#include <ibrcommon/net/socketstream.h>
#include <vector>

class BundleStream : public dtn::api::Client
{
public:
	BundleStream(ibrcommon::socketstream &stream, size_t min_chunk_size, size_t max_chunk_size, const std::string &app = "stream", const dtn::data::EID &group = dtn::data::EID(), bool wait_seq_zero = false);
	virtual ~BundleStream();

	BundleStreamBuf& rdbuf();
	dtn::data::Bundle& base();

	/**
	 * Enable automatic flush using Status Reports
	 */
	void setAutoFlush(bool val);

	/**
	 * Set the timeout for receiving bundles
	 */
	void setReceiveTimeout(unsigned int timeout);

protected:
	/**
	 * @see dtn::api::Client::received()
	 */
	virtual void received(const dtn::data::Bundle &b);

private:
	ibrcommon::socketstream &_stream;
	StreamBundle _chunk;
	BundleStreamBuf _buf;
	bool _auto_flush;
	dtn::data::BundleID _last_delivery_ack;
};

#endif /* BUNDLESTREAM_H_ */
