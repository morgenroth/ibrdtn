/*
 * Logger.h
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

#ifndef LOGGER_H_
#define LOGGER_H_

//#define __IBRCOMMON_MULTITHREADED__

#include <ibrcommon/thread/Queue.h>
#include <ibrcommon/thread/Thread.h>
#include <ibrcommon/data/File.h>
#include <fstream>
#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <list>

/**
 * @file Logger.h
 *
 * This file provides common logging facilities. For easy usage it provides some defines and is usable
 * like a standard output stream. It is fully configurable, can log to all standard output streams and
 * to the system syslog.
 *
 * Provided tags:
 *  emergency, alert, critical, error, warning, notice, info
 *
 * Example log message with info tag:
 * \code
 * IBRCOMMON_LOGGER(info) << "Some output..." << IBRCOMMON_ENDL;
 * \endcode
 *
 * Example debug with verbose level 42:
 * \code
 * IBRCOMMON_LOGGER_DEBUG(42) << "some debugging output" << IBRCOMMON_ENDL;
 * \endcode
 *
 * To check the current verbose level in the code use the IBRCOMMON_LOGGER_LEVEL define like this:
 * \code
 * if (IBRCOMMON_LOGGER_LEVEL > 42)
 * {
 * 		do something...
 * }
 * \endcode
 *
 */

#define IBRCOMMON_LOGGER_LEVEL \
	ibrcommon::Logger::getVerbosity()

#define IBRCOMMON_LOGGER(level) \
	{ \
		ibrcommon::Logger log = ibrcommon::Logger::level(); \
		log

#define IBRCOMMON_LOGGER_DEBUG(verbosity) \
	if (ibrcommon::Logger::getVerbosity() >= verbosity) \
	{ \
		ibrcommon::Logger log = ibrcommon::Logger::debug(verbosity); \
		log

#define IBRCOMMON_LOGGER_ENDL \
		std::flush; \
		log.print(); \
	}

#define IBRCOMMON_LOGGER_ex(level) \
	IBRCOMMON_LOGGER(level) << __PRETTY_FUNCTION__ << ": "

#define IBRCOMMON_LOGGER_DEBUG_ex(verbosity) \
	IBRCOMMON_LOGGER_DEBUG(verbosity) << __FILE__ << ":" << __LINE__ << " in " << __PRETTY_FUNCTION__ << ": "

namespace ibrcommon
{
	/**
	 * @class Logger
	 *
	 * The Logger class is the heart of the logging framework.
	 */
	class Logger : public std::stringstream
	{
	public:
		enum LogOptions
		{
			LOG_NONE =		0,
			LOG_DATETIME =	1 << 0,	/* print date/time on log messages */
			LOG_HOSTNAME = 	1 << 1, /* print hostname on log messages */
			LOG_LEVEL =		1 << 2,	/* print the log level on log messages */
			LOG_TIMESTAMP =	1 << 3	/* print timestamp on log messages */
		};

		enum LogLevel
		{
			LOGGER_EMERG =		1 << 0,	/* system is unusable */
			LOGGER_ALERT =		1 << 1,	/* action must be taken immediately */
			LOGGER_CRIT =		1 << 2,	/* critical conditions */
			LOGGER_ERR =		1 << 3,	/* error conditions */
			LOGGER_WARNING = 	1 << 4,	/* warning conditions */
			LOGGER_NOTICE = 	1 << 5,	/* normal but significant condition */
			LOGGER_INFO = 		1 << 6,	/* informational */
			LOGGER_DEBUG = 		1 << 7,	/* debug-level messages */
			LOGGER_ALL =		0xff
		};

		Logger(const Logger&);
		virtual ~Logger();

		static Logger emergency();
		static Logger alert();
		static Logger critical();
		static Logger error();
		static Logger warning();
		static Logger notice();
		static Logger info();
		static Logger debug(int verbosity);

		/**
		 * Set the global verbosity of the logger.
		 * @param verbosity A verbosity level as number. Higher value leads to more output.
		 */
		static void setVerbosity(const int verbosity);

		/**
		 * Get the global verbosity of the logger.
		 * @return The verbosity level as number. Higher value leads to more output.
		 */
		static int getVerbosity();

		/**
		 * Add a standard output stream to the logging framework.
		 * @param stream Standard output stream
		 * @param logmask This mask specify what will be written to this stream. You can combine options with the or function.
		 * @param options This mask specify what will be added to each log message. You can combine options with the or function.
		 */
		static void addStream(std::ostream &stream, const unsigned char logmask = LOGGER_INFO, const unsigned char options = LOG_NONE);

		/**
		 * Set the logfile for the logging framework.
		 * @param logfile The file to log into.
		 * @param logmask This mask specify what will be written to this stream. You can combine options with the or function.
		 * @param options This mask specify what will be added to each log message. You can combine options with the or function.
		 */
		static void setLogfile(const ibrcommon::File &logfile, const unsigned char logmask = LOGGER_INFO, const unsigned char options = LOG_NONE);

		/**
		 * Enable logging message to the system syslog.
		 * @param name The naming prefix for all log messages.
		 * @param option Syslog specific options. @see syslog.h
		 * @param facility Syslog facility. @see syslog.h
		 * @param logmask This mask specify what will be written to the syslog. You can combine options with the or function.
		 */
		static void enableSyslog(const char *name, int option, int facility, const unsigned char logmask = LOGGER_INFO);

		/**
		 * enable the asynchronous logging
		 * This starts a seperate thread and a thread-safe queue to
		 * queue all logging messages first and call the log routine by
		 * the thread. This option is nessacary, if the stream to log into
		 * are not thread-safe by itself.
		 */
		static void enableAsync();

		/**
		 * Enables the internal ring-buffer.
		 * @param size The size of the buffer.
		 */
		static void enableBuffer(size_t size);

		/**
		 * Write the buffer to a stream
		 * @param
		 */
		static void writeBuffer(std::ostream&);

		/**
		 * stops the asynchronous logging thread
		 * you need to call this before your programm is going down, if you have
		 * called enableAsync() before. 
		 */
		static void stop();

		/**
		 * Print the log message to the log output (or queue it in the writer)
		 */
		void print();
		
		/**
		 * Reload the logger. This re-opens the logfile for output.
		 */
		static void reload();

	private:
		Logger(LogLevel level, int debug_verbosity = 0);

		class LoggerOutput
		{
		public:
			LoggerOutput(std::ostream &stream, const unsigned char logmask, const unsigned char options);
			virtual ~LoggerOutput();

			void log(const Logger &log);

			std::ostream &_stream;
			unsigned char _level;
			unsigned char _options;
		};
		
		class LogWriter : public ibrcommon::JoinableThread
		{
		public:
			LogWriter();
			virtual ~LogWriter();

			void log(Logger &logger);

			/**
			 * Set the global verbosity of the logger.
			 * @param verbosity A verbosity level as number. Higher value leads to more output.
			 */
			void setVerbosity(const int verbosity);

			/**
			 * Get the global verbosity of the logger.
			 * @return The verbosity level as number. Higher value leads to more output.
			 */
			int getVerbosity();

			/**
			 * Add a standard output stream to the logging framework.
			 * @param stream Standard output stream
			 * @param logmask This mask specify what will be written to this stream. You can combine options with the or function.
			 * @param options This mask specify what will be added to each log message. You can combine options with the or function.
			 */
			void addStream(std::ostream &stream, const unsigned char logmask = LOGGER_INFO, const unsigned char options = LOG_NONE);

			/**
			 * Set the logfile for the logging framework.
			 * @param logfile The file to log into.
			 * @param logmask This mask specify what will be written to this stream. You can combine options with the or function.
			 * @param options This mask specify what will be added to each log message. You can combine options with the or function.
			 */
			void setLogfile(const ibrcommon::File &logfile, const unsigned char logmask = LOGGER_INFO, const unsigned char options = LOG_NONE);

			/**
			 * Enable logging message to the system syslog.
			 * @param name The naming prefix for all log messages.
			 * @param option Syslog specific options. @see syslog.h
			 * @param facility Syslog facility. @see syslog.h
			 * @param logmask This mask specify what will be written to the syslog. You can combine options with the or function.
			 */
			void enableSyslog(const char *name, int option, int facility, const unsigned char logmask = LOGGER_INFO);

			/**
			 * enable the asynchronous logging
			 * This starts a seperate thread and a thread-safe queue to
			 * queue all logging messages first and call the log routine by
			 * the thread. This option is nessacary, if the stream to log into
			 * are not thread-safe by itself.
			 */
			void enableAsync();

			/**
			 * Enables the internal ring-buffer.
			 * @param size The size of the buffer.
			 */
			void enableBuffer(size_t size);

			/**
			 * Reload the logger. This re-opens the logfile for output.
			 */
			void reload();

			/**
			 * Write the buffer to a stream
			 * @param
			 */
			void writeBuffer(std::ostream&, const unsigned char logmask, const unsigned char options);

		protected:
			void flush();
			void run();
			void __cancellation();

		private:
			/**
			 * Flush the log message to the output/syslog.
			 */
			void flush(const Logger &logger);

			int _verbosity;
			bool _syslog;
			unsigned char _syslog_mask;

			ibrcommon::Queue<Logger> _queue;
			bool _use_queue;
			std::list<LoggerOutput> _logger;

			ibrcommon::Mutex _buffer_mutex;
			size_t _buffer_size;
			std::list<Logger> *_buffer;

			ibrcommon::Mutex _logfile_mutex;
			ibrcommon::File _logfile;
			std::ofstream _logfile_stream;
			LoggerOutput *_logfile_output;
			unsigned char _logfile_logmask;
			unsigned char _logfile_options;
		};

		LogLevel _level;
		int _debug_verbosity;
		struct timeval _logtime;

		static LogWriter _logwriter;
	};
}

#endif /* LOGGER_H_ */
