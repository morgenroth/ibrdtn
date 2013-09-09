/*
 * iobuffer.h
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

#ifndef IOBUFFER_H_
#define IOBUFFER_H_

#include "ibrcommon/Exceptions.h"
#include <streambuf>
#include "ibrcommon/thread/Conditional.h"
#include <vector>

namespace ibrcommon
{
	/**
	 * This class provided a way to exchange buffered data between two threads
	 * where one thread is writing to the buffer and the other is reading from.
	 */
	class iobuffer : public std::basic_streambuf<char, std::char_traits<char> >
	{
	public:
		iobuffer(const size_t buffer = 2048);
		virtual ~iobuffer();

		/**
		 * finalize the stream an set the EOF flag
		 */
		void finalize();

	protected:
		virtual int sync();
		virtual std::char_traits<char>::int_type overflow(std::char_traits<char>::int_type = std::char_traits<char>::eof());
		virtual std::char_traits<char>::int_type underflow();

	private:
		ibrcommon::Conditional _data_cond;

		bool _eof;

		size_t _data_size;

		// length of the data buffer
		size_t _buf_size;

		// input buffer
		std::vector<char> _input_buf;

		// interim buffer
		std::vector<char> _interim_buf;

		// output buffer
		std::vector<char> _output_buf;
	};
}

#endif /* IOBUFFER_H_ */
