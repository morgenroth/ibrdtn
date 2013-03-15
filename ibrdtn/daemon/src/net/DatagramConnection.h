/*
 * DatagramConnection.h
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

#ifndef DATAGRAMCONNECTION_H_
#define DATAGRAMCONNECTION_H_

#include "net/ConvergenceLayer.h"
#include "net/DatagramService.h"
#include <ibrcommon/thread/Thread.h>
#include <ibrcommon/thread/Queue.h>
#include <ibrcommon/thread/Conditional.h>
#include <streambuf>
#include <iostream>
#include <stdint.h>

namespace dtn
{
	namespace net
	{
		class DatagramConnection;

		class DatagramConnectionCallback
		{
		public:
			virtual ~DatagramConnectionCallback() {};
			virtual void callback_send(DatagramConnection &connection, const char &flags, const unsigned int &seqno, const std::string &destination, const char *buf, int len) throw (DatagramException) = 0;
			virtual void callback_ack(DatagramConnection &connection, const unsigned int &seqno, const std::string &destination) throw (DatagramException) = 0;

			virtual void connectionUp(const DatagramConnection *conn) = 0;
			virtual void connectionDown(const DatagramConnection *conn) = 0;
		};

		class DatagramConnection : public ibrcommon::DetachedThread
		{
		public:
			static const std::string TAG;

			DatagramConnection(const std::string &identifier, const DatagramService::Parameter &params, DatagramConnectionCallback &callback);
			virtual ~DatagramConnection();

			void run() throw ();
			void setup() throw ();
			void finally() throw ();

			virtual void __cancellation() throw ();

			void shutdown();

			const std::string& getIdentifier() const;

			/**
			 * Queue job for delivery to another node
			 * @param job
			 */
			void queue(const ConvergenceLayer::Job &job);

			/**
			 * queue data for delivery to the stream
			 * @param buf
			 * @param len
			 */
			void queue(const char &flags, const unsigned int &seqno, const char *buf, int len);

			/**
			 * This method is called by the DatagramCL, if an ACK is received.
			 * @param seq
			 */
			void ack(const unsigned int &seqno);

			/**
			 * Assign a peer EID to this connection
			 * @param peer The EID of this peer.
			 */
			void setPeerEID(const dtn::data::EID &peer);

		private:
			enum SEND_FLOW {
				SEND_IDLE,
				SEND_WAIT_ACK,
				SEND_NEXT,
				SEND_ERROR
			} _send_state;

			enum RECV_FLOW {
				RECV_IDLE,
				RECV_HEAD,
				RECV_TRANSMISSION,
				RECV_ERROR
			} _recv_state;

			class Stream : public std::basic_streambuf<char, std::char_traits<char> >, public std::iostream
			{
			public:
				Stream(DatagramConnection &conn, const size_t maxmsglen);
				virtual ~Stream();

				/**
				 * Queueing data received from the CL worker thread for the LOWPANConnection
				 * @param buf Buffer with received data
				 * @param len Length of the buffer
				 */
				void queue(const char *buf, int len) throw (DatagramException);

				/**
				 * Close the stream to terminate all blocking
				 * calls on it.
				 */
				void close();

			protected:
				virtual int sync();
				virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());
				virtual std::char_traits<char>::int_type underflow();

			private:
				// buffer size and maximum message size
				const size_t _buf_size;

				// will be set to true if the next segment is the last
				// of the bundle
				bool _last_segment;

				// buffer for incoming data to queue
				// the underflow method will block until
				// this buffer contains any data
				char *_queue_buf;

				// the number of bytes available in the queue buffer
				int _queue_buf_len;

				// conditional to lock the queue buffer and the
				// corresponding length variable
				ibrcommon::Conditional _queue_buf_cond;

				// outgoing data from the upper layer is stored
				// here first and processed by the overflow() method
				char *_out_buf;

				// incoming data to deliver data to the upper layer
				// is stored in this buffer by the underflow() method
				char *_in_buf;

				// this variable is set to true to shutdown
				// this stream
				bool _abort;

				// callback to the corresponding connection object
				DatagramConnection &_callback;
			};

			class Sender : public ibrcommon::JoinableThread
			{
			public:
				Sender(DatagramConnection &conn, Stream &stream);
				~Sender();

				void run() throw ();
				void finally() throw ();
				void __cancellation() throw ();

				void clearQueue();

				ibrcommon::Queue<ConvergenceLayer::Job> queue;

			private:
				ConvergenceLayer::Job _current_job;
				DatagramConnection::Stream &_stream;

				// callback to the corresponding connection object
				DatagramConnection &_connection;
			};

			void stream_send(const char *buf, int len, bool last) throw (DatagramException);

			void adjust_rtt(float value);

			DatagramConnectionCallback &_callback;
			const std::string _identifier;
			DatagramConnection::Stream _stream;
			DatagramConnection::Sender _sender;

			ibrcommon::Conditional _ack_cond;
			size_t _last_ack;
			size_t _next_seqno;

			// stores the head of each connection
			// the head is hold back until at least a second
			// or the last segment was received
			char *_head_buf;
			size_t _head_len;

			const DatagramService::Parameter _params;

			size_t _avg_rtt;

			dtn::data::EID _peer_eid;
		};
	} /* namespace data */
} /* namespace dtn */
#endif /* DATAGRAMCONNECTION_H_ */
