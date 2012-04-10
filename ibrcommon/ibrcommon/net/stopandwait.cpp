/*
 * stopandwait.cpp
 *
 *  Created on: 08.11.2010
 *      Author: morgenro
 */

#include "ibrcommon/net/stopandwait.h"
#include <ibrcommon/thread/MutexLock.h>
#include <stdio.h>
#include <string.h>

namespace ibrcommon
{
	stopandwait::stopandwait(const size_t timeout, const size_t maxretry)
	 : _maxretry(maxretry), _out_seqno(1), _in_seqno(0), _ack_seqno(0), _count(0), _timeout(timeout)
	{
	}

	stopandwait::~stopandwait()
	{
	}

	void stopandwait::setTimeout(size_t timeout)
	{
		_timeout = timeout;
	}

	int stopandwait::__send(const char *buffer, const size_t length)
	{
		char sendbuf[length + 2];

		// set message type
		sendbuf[0] = 1; 	// 1 = DATA

		// add sequence number
		sendbuf[1] = _out_seqno; _out_seqno++;

		// copy the buffer
		char *sendptr = ((char*)&sendbuf) + 2;
		::memcpy(sendptr, buffer, length);

		// send the buffer
		if (__send_impl((char*)&sendbuf, length + 2) != 0)
		{
			// ERROR!
			return -1;
		}

		// wait for ACK
		ibrcommon::MutexLock l(_ack_cond);
		while (_ack_seqno != _out_seqno)
		{
			try {
				_ack_cond.wait(_timeout);
			} catch (const ibrcommon::Conditional::ConditionalAbortException &ex) {
				if (ex.reason == ibrcommon::Conditional::ConditionalAbortException::COND_TIMEOUT)
				{
					// retransmission
					_count++;

					// abort if the number of retries exceeds the maximum number
					if ((_maxretry > 0) && (_count > _maxretry))
					{
						return -1;
					}

					// resend the message
					if (__send_impl((char*)&sendbuf, length + 2) != 0)
					{
						// ERROR!
						return -1;
					}
				}
				else
				{
					// ERROR or ABORT
					return -1;
				}
			}
		}
	}

	int stopandwait::__recv(char *buffer, size_t &length)
	{
		char *bufferptr = buffer;

		while (true)
		{
			int ret = __recv_impl(bufferptr, length);
			if (ret != 0) return ret;

			// message type
			char msgtype = *bufferptr; bufferptr++;

			// read seqnr
			u_int8_t seqno = *bufferptr; bufferptr++;

			// check for double received messages
			if (seqno > _in_seqno)
			{
				_in_seqno = seqno;

				switch (msgtype)
				{
					case 1:		// DATA
					{
						::memcpy(buffer, bufferptr, length - 2);
						length -= 2;
						return ret;
					}

					case 2:		// ACK
					{
						ibrcommon::MutexLock l(_ack_cond);
						_ack_seqno = seqno;
						_ack_cond.signal(true);
						break;
					}
				}
			}
		}

		return -1;
	}
}
