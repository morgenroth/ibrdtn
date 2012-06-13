/*
 * lowpanstream.h
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

#ifndef LOWPANSTREAM_H_
#define LOWPANSTREAM_H_

#include <ibrcommon/thread/Conditional.h>
#include <streambuf>
#include <iostream>
#include <stdint.h>

namespace ibrcommon
{
	class lowpanstream_callback
	{
	public:
		virtual ~lowpanstream_callback() { };
		/**
		 * Callback interface implementation from LoWPAN CL
		 * @see LOWPANConvergenceLayer
		 */
		virtual void send_cb(char *buf, int len, unsigned int address) = 0;
	};

	class lowpanstream : public std::basic_streambuf<char, std::char_traits<char> >, public std::iostream
	{
	public:
		/**
		 * Size of the input and output buffers.
		 */
		static const size_t BUFF_SIZE = 113;

		lowpanstream(lowpanstream_callback &callback, unsigned int address);
		virtual ~lowpanstream();

		/**
		 * Queueing data received from the CL worker thread for the LOWPANConnection
		 * @param buf Buffer with received data
		 * @param len Length of the buffer
		 */
		void queue(char *buf, int len);

		void abort();

	protected:
		virtual int sync();
		virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());
		virtual std::char_traits<char>::int_type underflow();

		unsigned int _address;

	private:
		bool _in_first_segment;
		char _out_stat;

		// Input buffer
		char *in_buf_;
		int in_buf_len;
		bool in_buf_free;

		// Output buffer
		char *out_buf_;
		char *out2_buf_;

		// sequence number
		uint8_t in_seq_num_;
		uint8_t out_seq_num_;
		long out_seq_num_global;

		bool _abort;

		lowpanstream_callback &callback;

		ibrcommon::Conditional in_buf_cond;
	};
}
#endif /* LOWPANSTREAM_H_ */
