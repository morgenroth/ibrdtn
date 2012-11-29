/*
 * stopandwait.h
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

#ifndef STOPANDWAIT_H_
#define STOPANDWAIT_H_

#include <ibrcommon/thread/Conditional.h>
#include <stdint.h>

namespace ibrcommon
{
	class stopandwait
	{
	public:
		stopandwait(const size_t timeout, const size_t maxretry = 0);
		virtual ~stopandwait();

		void setTimeout(size_t timeout);

	protected:
		int __send(const char *buffer, const size_t length);
		int __recv(char *buffer, size_t &length);

		virtual int __send_impl(const char *buffer, const size_t length)  = 0;
		virtual int __recv_impl(char *buffer, size_t &length) = 0;

	private:
		size_t _maxretry;
		uint8_t _out_seqno;
		uint8_t _in_seqno;
		uint8_t _ack_seqno;

		unsigned int _count;
		size_t _timeout;
		ibrcommon::Conditional _ack_cond;
	};
}

#endif /* STOPANDWAIT_H_ */
