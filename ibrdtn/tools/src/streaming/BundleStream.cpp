/*
 * BundleStream.cpp
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

#include <streaming/BundleStream.h>
#include <ibrdtn/data/StatusReportBlock.h>

BundleStream::BundleStream(ibrcommon::socketstream &stream, size_t min_chunk_size, size_t max_chunk_size, const std::string &app, const dtn::data::EID &group, bool wait_seq_zero)
 : dtn::api::Client(app, group, stream), _stream(stream), _buf(*this, _chunk, min_chunk_size, max_chunk_size, wait_seq_zero), _auto_flush(false)
{}

BundleStream::~BundleStream() {}

BundleStreamBuf& BundleStream::rdbuf()
{
	return _buf;
}

dtn::data::Bundle& BundleStream::base()
{
	return _chunk;
}

void BundleStream::setAutoFlush(bool val)
{
	_auto_flush = val;
	_buf.setRequestAck(val);
}

void BundleStream::setReceiveTimeout(unsigned int timeout)
{
	_buf.setReceiveTimeout(timeout);
}

void BundleStream::received(const dtn::data::Bundle &b)
{
	// check if the received bundle contains an administrative record
	if (b.get(dtn::data::PrimaryBlock::APPDATA_IS_ADMRECORD))
	{
		// get the payload block of the bundle
		const dtn::data::PayloadBlock &payload = b.find<dtn::data::PayloadBlock>();

		try {
			// try to decode as status report
			dtn::data::StatusReportBlock report;
			report.read(payload);

			// check if the received report matches a newer bundle
			if ((_last_delivery_ack.source == dtn::data::EID()) || (report.bundleid > _last_delivery_ack))
			{
				// store the received ack
				_last_delivery_ack = report.bundleid;

				// flush the buffer
				_buf.flush();
			}

			return;
		} catch (const dtn::data::StatusReportBlock::WrongRecordException&) {
			// this is not a status report
		}
		return;
	}

	// forward received bundles to the BundleStreamBuf
	_buf.received(b);
}
