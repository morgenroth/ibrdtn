/*
 * SyslogStream.cpp
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

#include "ibrcommon/config.h"
#include "ibrcommon/SyslogStream.h"
#include <syslog.h>

namespace ibrcommon
{
	std::ostream &slog = SyslogStream::getStream();

	SyslogStream::SyslogStream() : m_buf( BUF_SIZE+1 ), _prio(SYSLOG_INFO)
	{
		setp( &m_buf[0], &m_buf[0] + (m_buf.size()-1) );
	}

	SyslogStream::~SyslogStream()
	{
	}

	void SyslogStream::open(char *name, int option, int facility)
	{
		::openlog(name, option, facility);
	}

	std::ostream& SyslogStream::getStream()
	{
		static SyslogStream syslogbuf;
		static std::ostream stream(&syslogbuf);

		return stream;
	}

	int SyslogStream::sync()
	{
		if( pptr() > pbase() )
			writeToDevice();
		return 0; // 0 := Ok
	}

	void SyslogStream::setPriority(const SyslogPriority &prio)
	{
		_prio = prio;
	}

	void SyslogStream::writeToDevice()
	{
		*pptr() = 0; // End Of String
		::syslog( _prio, "%s", pbase() );
		setp( &m_buf[0], &m_buf[0] + (m_buf.size()-1) );
	}

	std::ostream &operator<<(std::ostream &stream, const SyslogPriority &prio)
	{
		SyslogStream *slog = dynamic_cast<SyslogStream*>(stream.rdbuf());

		if (slog != NULL)
		{
			slog->setPriority(prio);
		}
		else
		{
			const char data = prio;
			stream.write(&data, sizeof(char));
		}

		return stream;
	}
}
