/*
 * SyslogStream.h
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

#ifndef IBRCOMMON_SYSLOGSTREAM_H
#define	IBRCOMMON_SYSLOGSTREAM_H

#include <vector>
#include <syslog.h>
#include <iostream>
#include <string.h>

namespace ibrcommon
{
	enum SyslogPriority
	{
		SYSLOG_EMERG =	LOG_EMERG,	/* system is unusable */
		SYSLOG_ALERT =	LOG_ALERT,	/* action must be taken immediately */
		SYSLOG_CRIT =	LOG_CRIT,	/* critical conditions */
		SYSLOG_ERR =	LOG_ERR,	/* error conditions */
		SYSLOG_WARNING =    LOG_WARNING,	/* warning conditions */
		SYSLOG_NOTICE =	LOG_NOTICE,	/* normal but significant condition */
		SYSLOG_INFO =	LOG_INFO,	/* informational */
		SYSLOG_DEBUG =	LOG_DEBUG	/* debug-level messages */
	};

	std::ostream &operator<<(std::ostream &stream, const SyslogPriority &prio);

	class SyslogStream : public std::streambuf
	{
	private:
		enum { BUF_SIZE = 256 };

	public:
		SyslogStream();
		virtual ~SyslogStream();

		static std::ostream& getStream();
		static void open(char *name, int option, int facility);

		void setPriority(const SyslogPriority &prio);

	protected:
		virtual int sync();

	private:
		void writeToDevice();
		std::vector< char_type > m_buf;
		SyslogPriority _prio;
	};

	extern std::ostream &slog;
}

#endif	/* _SYSLOGSTREAM_H */

