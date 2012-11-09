/*
 * DatagramConnection.cpp
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

#include "net/DatagramConnection.h"
#include "net/BundleReceivedEvent.h"
#include "core/BundleEvent.h"
#include "net/TransferCompletedEvent.h"
#include "net/TransferAbortedEvent.h"
#include "routing/RequeueBundleEvent.h"
#include "core/BundleCore.h"

#include <ibrdtn/utils/Utils.h>
#include <ibrdtn/data/Serializer.h>

#include <ibrcommon/TimeMeasurement.h>
#include <ibrcommon/Logger.h>
#include <string.h>

#define AVG_RTT_WEIGHT 0.875

namespace dtn
{
	namespace net
	{
		DatagramConnection::DatagramConnection(const std::string &identifier, const DatagramService::Parameter &params, DatagramConnectionCallback &callback)
		 : _callback(callback), _identifier(identifier), _stream(*this, params.max_msg_length, params.max_seq_numbers), _sender(*this, _stream), _last_ack(-1), _wait_ack(-1), _params(params), _avg_rtt(params.initial_timeout)
		{
		}

		DatagramConnection::~DatagramConnection()
		{
			// do not destroy this instance as long as
			// the sender thread is running
			_sender.join();
		}

		void DatagramConnection::shutdown()
		{
			try {
				// abort the connection thread
				ibrcommon::DetachedThread::stop();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_DEBUG(50) << "DatagramConnection::shutdown(): ThreadException (" << ex.what() << ")" << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void DatagramConnection::__cancellation() throw ()
		{
			// close the stream
			try {
				_stream.close();
			} catch (const ibrcommon::Exception&) { };
		}

		void DatagramConnection::run() throw ()
		{
			try {
				while(_stream.good())
				{
					dtn::data::DefaultDeserializer deserializer(_stream);
					dtn::data::Bundle bundle;
					deserializer >> bundle;
					
					// validate the bundle
					dtn::core::BundleCore::getInstance().validate(bundle);

					IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConnection::run"<< IBRCOMMON_LOGGER_ENDL;

					// raise default bundle received event
					dtn::net::BundleReceivedEvent::raise(_peer_eid, bundle, false, true);
				}
			} catch (const dtn::InvalidDataException &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "Received an invalid bundle: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			} catch (std::exception &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "Thread died: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void DatagramConnection::setup() throw ()
		{
			_callback.connectionUp(this);
			_sender.start();
		}

		void DatagramConnection::finally() throw ()
		{
			IBRCOMMON_LOGGER_DEBUG(60) << "DatagramConnection down" << IBRCOMMON_LOGGER_ENDL;

			try {
				ibrcommon::MutexLock l(_ack_cond);
				_ack_cond.abort();
			} catch (const std::exception&) { };

			try {
				// shutdown the sender thread
				_sender.stop();

				// wait until all operations are stopped
				_sender.join();
			} catch (const std::exception&) { };

			try {
				// remove this connection from the connection list
				_callback.connectionDown(this);
			} catch (const ibrcommon::MutexException&) { };

			// clear the queue
			_sender.clearQueue();
		}

		const std::string& DatagramConnection::getIdentifier() const
		{
			return _identifier;
		}

		/**
		 * Queue job for delivery to another node
		 * @param job
		 */
		void DatagramConnection::queue(const ConvergenceLayer::Job &job)
		{
			IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConnection::queue"<< IBRCOMMON_LOGGER_ENDL;
			_sender.queue.push(job);
		}

		/**
		 * queue data for delivery to the stream
		 * @param buf
		 * @param len
		 */
		void DatagramConnection::queue(const char &flags, const unsigned int &seqno, const char *buf, int len)
		{
			_stream.queue(flags, seqno, buf, len);

			if (_params.flowcontrol == DatagramService::FLOW_STOPNWAIT)
			{
				// send ack for this message
				_callback.callback_ack(*this, seqno, getIdentifier());
			}
		}

		void DatagramConnection::ack(const unsigned int &seqno)
		{
			ibrcommon::MutexLock l(_ack_cond);
			if (_wait_ack == seqno)
			{
				_last_ack = seqno;
				_ack_cond.signal(true);
				IBRCOMMON_LOGGER_DEBUG(20) << "DatagramConnection: ack received " << _last_ack << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void DatagramConnection::setPeerEID(const dtn::data::EID &peer)
		{
			_peer_eid = peer;
		}

		void DatagramConnection::stream_send(const char &flags, const unsigned int &seqno, const char *buf, int len) throw (DatagramException)
		{
			// measure the time until the ack is received
			ibrcommon::TimeMeasurement tm;

			_wait_ack = seqno;

			// max. 5 retries
			for (int i = 0; i < 5; i++)
			{
				// send the datagram
				_callback.callback_send(*this, flags, seqno, getIdentifier(), buf, len);

				if (_params.flowcontrol == DatagramService::FLOW_STOPNWAIT)
				{
					tm.start();

					// set timeout to twice the average round-trip-time
					struct timespec ts;
					ibrcommon::Conditional::gettimeout(_avg_rtt * 2, &ts);

					try {
						// wait here for an ACK
						ibrcommon::MutexLock l(_ack_cond);
						while (_last_ack != _wait_ack)
						{
							_ack_cond.wait(&ts);
						}

						// success!
						// stop the measurement
						tm.stop();

						// adjust the average rtt
						adjust_rtt(tm.getMilliseconds());

						return;
					} catch (const ibrcommon::Conditional::ConditionalAbortException &e) {
						// fail -> increment the future timeout
						adjust_rtt(_avg_rtt * 2);

						// retransmit the frame
					}
				}
				else
				{
					// success by default
					return;
				}
			}

			// transmission failed - abort the stream
			IBRCOMMON_LOGGER_DEBUG(20) << "DatagramConnection::stream_send: transmission failed - abort the stream" << IBRCOMMON_LOGGER_ENDL;
			throw DatagramException("transmission failed - abort the stream");
		}

		void DatagramConnection::adjust_rtt(float value)
		{
			// convert current avg to float
			float new_rtt = _avg_rtt;

			// calculate average
			new_rtt = (new_rtt * (1 - AVG_RTT_WEIGHT)) + (AVG_RTT_WEIGHT * value);

			// assign the new value
			_avg_rtt = new_rtt;
		}

		DatagramConnection::Stream::Stream(DatagramConnection &conn, const size_t maxmsglen, const unsigned int maxseqno)
		 : std::iostream(this), _buf_size(maxmsglen), _maxseqno(maxseqno), _in_state(DatagramService::SEGMENT_FIRST), _out_state(DatagramService::SEGMENT_FIRST),
		   _queue_buf(new char[_buf_size]), _queue_buf_len(0), _out_buf(new char[_buf_size]), _in_buf(new char[_buf_size]),
		   in_seq_num_(0), out_seq_num_(0), _abort(false), _callback(conn)
		{
			// Initialize get pointer. This should be zero so that underflow
			// is called upon first read.
			setg(0, 0, 0);

			// mark the buffer for outgoing data as free
			// the +1 sparse the first byte in the buffer and leave room
			// for the processing flags of the segment
			setp(_out_buf, _out_buf + _buf_size - 1);
		}

		DatagramConnection::Stream::~Stream()
		{
			delete[] _queue_buf;
			delete[] _out_buf;
			delete[] _in_buf;
		}

		void DatagramConnection::Stream::queue(const char &flags, const unsigned int &seqno, const char *buf, int len)
		{
			ibrcommon::MutexLock l(_queue_buf_cond);
			if (_abort) throw DatagramException("stream aborted");

			IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConnection::Stream::queue(): Received frame sequence number: " << seqno << IBRCOMMON_LOGGER_ENDL;

			// Check if the sequence number is what we expect
			if ((_callback._params.seq_check) && (in_seq_num_ != seqno))
			{
				IBRCOMMON_LOGGER(error) << "Received frame with out of bound sequence number (" << seqno << " expected " << in_seq_num_ << ")"<< IBRCOMMON_LOGGER_ENDL;
				_abort = true;
				_queue_buf_cond.signal();

				if (flags & DatagramService::SEGMENT_FIRST)
				{
					throw DatagramException("out of bound exception - re-initiate the connection");
				}
				return;
			}

			// check if this is the first segment since we expect a first segment
			if ((_in_state & DatagramService::SEGMENT_FIRST) && (!(flags & DatagramService::SEGMENT_FIRST)))
			{
				IBRCOMMON_LOGGER(error) << "Received frame with wrong segment mark"<< IBRCOMMON_LOGGER_ENDL;
				_abort = true;
				_queue_buf_cond.signal();
				return;
			}
			// check if this is a second first segment without any previous last segment
			else if ((_in_state == DatagramService::SEGMENT_MIDDLE) && (flags & DatagramService::SEGMENT_FIRST))
			{
				IBRCOMMON_LOGGER(error) << "Received frame with wrong segment mark"<< IBRCOMMON_LOGGER_ENDL;
				_abort = true;
				_queue_buf_cond.signal();
				return;
			}

			if (flags & DatagramService::SEGMENT_FIRST)
			{
				IBRCOMMON_LOGGER_DEBUG(45) << "DatagramConnection: first segment received" << IBRCOMMON_LOGGER_ENDL;
			}

			// if this is the last segment then...
			if (flags & DatagramService::SEGMENT_LAST)
			{
				IBRCOMMON_LOGGER_DEBUG(45) << "DatagramConnection: last segment received" << IBRCOMMON_LOGGER_ENDL;

				// ... expect a first segment as next
				_in_state = DatagramService::SEGMENT_FIRST;
			}
			else
			{
				// if this is not the last segment we expect everything
				// but a first segment
				_in_state = DatagramService::SEGMENT_MIDDLE;
			}

			// increment the sequence number
			in_seq_num_ = (in_seq_num_ + 1) % _maxseqno;

			// wait until the buffer is free
			while (_queue_buf_len > 0)
			{
				_queue_buf_cond.wait();
			}

			// copy the new data into the buffer, but leave out the first byte (header)
			::memcpy(_queue_buf, buf, len);

			// store the buffer length
			_queue_buf_len = len;

			// notify waiting threads
			_queue_buf_cond.signal();
		}

		void DatagramConnection::Stream::close()
		{
			ibrcommon::MutexLock l(_queue_buf_cond);

			while (_queue_buf_len > 0)
			{
				_queue_buf_cond.wait();
			}

			_abort = true;
			_queue_buf_cond.abort();
		}

		int DatagramConnection::Stream::sync()
		{
			IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConnection::Stream::sync" << IBRCOMMON_LOGGER_ENDL;

			// Here we know we get the last segment. Mark it so.
			_out_state |= DatagramService::SEGMENT_LAST;

			int ret = std::char_traits<char>::eq_int_type(this->overflow(
					std::char_traits<char>::eof()), std::char_traits<char>::eof()) ? -1
					: 0;

			// initialize the first byte with SEGMENT_FIRST flag
			_out_state = DatagramService::SEGMENT_FIRST;

			return ret;
		}

		std::char_traits<char>::int_type DatagramConnection::Stream::overflow(std::char_traits<char>::int_type c)
		{
			if (_abort) throw DatagramException("stream aborted");

			char *ibegin = _out_buf;
			char *iend = pptr();

			IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConnection::Stream::overflow" << IBRCOMMON_LOGGER_ENDL;

			// mark the buffer for outgoing data as free
			// the +1 sparse the first byte in the buffer and leave room
			// for the processing flags of the segment
			setp(_out_buf, _out_buf + _buf_size - 1);

			if (!std::char_traits<char>::eq_int_type(c, std::char_traits<char>::eof()))
			{
				*iend++ = std::char_traits<char>::to_char_type(c);
			}

			// bytes to send
			size_t bytes = (iend - ibegin);

			// if there is nothing to send, just return
			if (bytes == 0)
			{
				IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConnection::Stream::overflow() nothing to sent" << IBRCOMMON_LOGGER_ENDL;
				return std::char_traits<char>::not_eof(c);
			}

			// Send segment to CL, use callback interface
			_callback.stream_send(_out_state, out_seq_num_, _out_buf, bytes);

			// increment the sequence number for outgoing segments
			out_seq_num_ = (out_seq_num_ + 1) % _maxseqno;

			// set the segment state to middle
			_out_state = DatagramService::SEGMENT_MIDDLE;

			return std::char_traits<char>::not_eof(c);
		}

		std::char_traits<char>::int_type DatagramConnection::Stream::underflow()
		{
			ibrcommon::MutexLock l(_queue_buf_cond);

			IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConnection::Stream::underflow"<< IBRCOMMON_LOGGER_ENDL;

			while (_queue_buf_len == 0)
			{
				if (_abort) throw ibrcommon::Exception("stream aborted");
				_queue_buf_cond.wait();
			}

			// copy the queue buffer to an internal buffer
			::memcpy(_in_buf,_queue_buf, _queue_buf_len);

			// Since the input buffer content is now valid (or is new)
			// the get pointer should be initialized (or reset).
			setg(_in_buf, _in_buf, _in_buf + _queue_buf_len);

			// mark the queue buffer as free
			_queue_buf_len = 0;
			_queue_buf_cond.signal();

			return std::char_traits<char>::not_eof((unsigned char) _in_buf[0]);
		}

		DatagramConnection::Sender::Sender(DatagramConnection &conn, Stream &stream)
		 : _stream(stream), _connection(conn)
		{
		}

		DatagramConnection::Sender::~Sender()
		{
		}

		void DatagramConnection::Sender::run() throw ()
		{
			try {
				while(_stream.good())
				{
					_current_job = queue.getnpop(true);
					dtn::data::DefaultSerializer serializer(_stream);

					IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConnection::Sender::run"<< IBRCOMMON_LOGGER_ENDL;

					dtn::storage::BundleStorage &storage = dtn::core::BundleCore::getInstance().getStorage();

					// read the bundle out of the storage
					const dtn::data::Bundle bundle = storage.get(_current_job._bundle);

					// Put bundle into stringstream
					serializer << bundle; _stream.flush();
					// raise bundle event
					dtn::net::TransferCompletedEvent::raise(_current_job._destination, bundle);
					dtn::core::BundleEvent::raise(bundle, dtn::core::BUNDLE_FORWARDED);
					_current_job.clear();
				}

				IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConnection::Sender::run stream destroyed"<< IBRCOMMON_LOGGER_ENDL;
			} catch (const ibrcommon::QueueUnblockedException &ex) {
				IBRCOMMON_LOGGER_DEBUG(50) << "DatagramConnection::Sender::run(): aborted" << IBRCOMMON_LOGGER_ENDL;
				return;
			} catch (std::exception &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "DatagramConnection::Sender terminated by exception: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}

			_connection.stop();
		}

		void DatagramConnection::Sender::clearQueue()
		{
			// requeue all bundles still queued
			try {
				while (true)
				{
					const ConvergenceLayer::Job job = queue.getnpop();

					// raise transfer abort event for all bundles without an ACK
					dtn::routing::RequeueBundleEvent::raise(job._destination, job._bundle);
				}
			} catch (const ibrcommon::QueueUnblockedException&) {
				// queue empty
			}
		}

		void DatagramConnection::Sender::finally() throw ()
		{
			// notify the aborted transfer of the last bundle
			if (_current_job._bundle != dtn::data::BundleID())
			{
				// put-back job on the queue
				queue.push(_current_job);
			}

			// abort all blocking operations on the stream
			_stream.close();
		}

		void DatagramConnection::Sender::__cancellation() throw ()
		{
			_stream.close();
			queue.abort();
		}
	} /* namespace data */
} /* namespace dtn */
