/*
 * iobuffer.h
 *
 *  Created on: 14.02.2011
 *      Author: morgenro
 */

#ifndef IOBUFFER_H_
#define IOBUFFER_H_

#include "ibrcommon/Exceptions.h"
#include <streambuf>
#include "ibrcommon/thread/Conditional.h"

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
		char *_input_buf;

		// interim buffer
		char *_interim_buf;

		// output buffer
		char *_output_buf;
	};
}

#endif /* IOBUFFER_H_ */
