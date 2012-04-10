/*
 * stopandwait.h
 *
 *  Created on: 08.11.2010
 *      Author: morgenro
 */

#include <ibrcommon/thread/Conditional.h>

#ifndef STOPANDWAIT_H_
#define STOPANDWAIT_H_

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
		u_int8_t _out_seqno;
		u_int8_t _in_seqno;
		u_int8_t _ack_seqno;

		unsigned int _count;
		size_t _timeout;
		ibrcommon::Conditional _ack_cond;
	};
}

#endif /* STOPANDWAIT_H_ */
