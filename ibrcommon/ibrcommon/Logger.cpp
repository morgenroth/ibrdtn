/*
 * Logger.cpp
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

#include "config.h"
#include "ibrcommon/Logger.h"
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/MutexLock.h>

#include <algorithm>
#include <sys/time.h>
#include <iomanip>
#include <vector>
#include <unistd.h>

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#endif

#ifdef ANDROID
#include <android/log.h>
#endif

namespace ibrcommon
{
	std::string Logger::_default_tag = "Core";
	Logger::LogWriter Logger::_logwriter;

	Logger::Logger(LogLevel level, const std::string &tag, int debug_verbosity)
	 : _level(level), _tag(tag), _debug_verbosity(debug_verbosity)
	{
		::gettimeofday(&_logtime, NULL);
	}

	Logger::Logger(const Logger &obj)
	 : _level(obj._level), _tag(obj._tag), _debug_verbosity(obj._debug_verbosity), _logtime(obj._logtime), _data(obj._data)
	{
	}

	Logger::~Logger()
	{
	}

	void Logger::setMessage(const std::string &data)
	{
		_data = data;
	}

	const std::string& Logger::str() const
	{
		return _data;
	}

	Logger Logger::emergency(const std::string &tag)
	{
		return Logger(LOGGER_EMERG, tag);
	}

	Logger Logger::alert(const std::string &tag)
	{
		return Logger(LOGGER_ALERT, tag);
	}

	Logger Logger::critical(const std::string &tag)
	{
		return Logger(LOGGER_CRIT, tag);
	}

	Logger Logger::error(const std::string &tag)
	{
		return Logger(LOGGER_ERR, tag);
	}

	Logger Logger::warning(const std::string &tag)
	{
		return Logger(LOGGER_WARNING, tag);
	}

	Logger Logger::notice(const std::string &tag)
	{
		return Logger(LOGGER_NOTICE, tag);
	}

	Logger Logger::info(const std::string &tag)
	{
		return Logger(LOGGER_INFO, tag);
	}

	Logger Logger::debug(const std::string &tag, int verbosity)
	{
		return Logger(LOGGER_DEBUG, tag, verbosity);
	}

	void Logger::setVerbosity(const int verbosity)
	{
		Logger::_logwriter.setVerbosity(verbosity);
	}

	void Logger::LogWriter::setVerbosity(const int verbosity)
	{
		_verbosity = verbosity;
	}

	unsigned char Logger::getLogMask()
	{
		return Logger::_logwriter.getLogMask();
	}

	int Logger::getVerbosity()
	{
		return Logger::_logwriter.getVerbosity();
	}

	unsigned char Logger::LogWriter::getLogMask() const
	{
		return _global_logmask;
	}

	int Logger::LogWriter::getVerbosity() const
	{
		return _verbosity;
	}

	void Logger::addStream(std::ostream &stream, const unsigned char logmask, const unsigned char options)
	{
		Logger::_logwriter.addStream(stream, logmask, options);
	}

	void Logger::setLogfile(const ibrcommon::File &logfile, const unsigned char logmask, const unsigned char options)
	{
		Logger::_logwriter.setLogfile(logfile, logmask, options);
	}

	void Logger::LogWriter::addStream(std::ostream &stream, const unsigned char logmask, const unsigned char options)
	{
		_global_logmask |= logmask;
		_logger.push_back( Logger::LoggerOutput(stream, logmask, options) );
	}

	void Logger::enableSyslog(const char *name, int option, int facility, const unsigned char logmask)
	{
		Logger::_logwriter.enableSyslog(name, option, facility, logmask);
	}

	void Logger::print()
	{
		Logger::_logwriter.log(*this);
	}

	void Logger::LogWriter::enableSyslog(const char *name, int option, int facility, const unsigned char logmask)
	{
#if ( defined HAVE_SYSLOG_H || defined ANDROID )
#ifdef HAVE_SYSLOG_H
		// init syslog
		::openlog(name, option, facility);
#endif
		_syslog = true;
		_syslog_mask = logmask;
		_global_logmask |= logmask;
#endif
	}

	void Logger::LogWriter::flush(const Logger &logger)
	{
		if (_verbosity >= logger._debug_verbosity)
		{
			for (std::list<LoggerOutput>::iterator iter = _logger.begin(); iter != _logger.end(); iter++)
			{
				LoggerOutput &output = (*iter);
				output.log(logger);
			}

			// output to logfile
			{
				ibrcommon::MutexLock l(_logfile_mutex);
				if (_logfile_output != NULL)
				{
					_logfile_output->log(logger);
				}
			}

#if ( defined HAVE_SYSLOG_H || defined ANDROID )
			// additionally log to the syslog/android logcat
			if (_syslog)
			{
				if (logger._level & _syslog_mask)
				{
#ifdef ANDROID
					std::string log_tag = Logger::_default_tag;
					if (logger._tag.length() > 0) {
						log_tag = logger._tag;
					}

					switch (logger._level)
					{
					case LOGGER_EMERG:
						__android_log_print(ANDROID_LOG_FATAL, log_tag.c_str(), "%s", logger.str().c_str());
						break;

					case LOGGER_ALERT:
						__android_log_print(ANDROID_LOG_FATAL, log_tag.c_str(), "%s", logger.str().c_str());
						break;

					case LOGGER_CRIT:
						__android_log_print(ANDROID_LOG_FATAL, log_tag.c_str(), "%s", logger.str().c_str());
						break;

					case LOGGER_ERR:
						__android_log_print(ANDROID_LOG_ERROR, log_tag.c_str(), "%s", logger.str().c_str());
						break;

					case LOGGER_WARNING:
						__android_log_print(ANDROID_LOG_WARN, log_tag.c_str(), "%s", logger.str().c_str());
						break;

					case LOGGER_NOTICE:
						__android_log_print(ANDROID_LOG_INFO, log_tag.c_str(), "%s", logger.str().c_str());
						break;

					case LOGGER_INFO:
						__android_log_print(ANDROID_LOG_INFO, log_tag.c_str(), "%s", logger.str().c_str());
						break;

					case LOGGER_DEBUG:
						__android_log_print(ANDROID_LOG_DEBUG, log_tag.c_str(), "%s", logger.str().c_str());
						break;

					default:
						__android_log_print(ANDROID_LOG_INFO, log_tag.c_str(), "%s", logger.str().c_str());
						break;
					}
#else
					switch (logger._level)
					{
					case LOGGER_EMERG:
						::syslog( LOG_EMERG, "%s", logger.str().c_str() );
						break;

					case LOGGER_ALERT:
						::syslog( LOG_ALERT, "%s", logger.str().c_str() );
						break;

					case LOGGER_CRIT:
						::syslog( LOG_CRIT, "%s", logger.str().c_str() );
						break;

					case LOGGER_ERR:
						::syslog( LOG_ERR, "%s", logger.str().c_str() );
						break;

					case LOGGER_WARNING:
						::syslog( LOG_WARNING, "%s", logger.str().c_str() );
						break;

					case LOGGER_NOTICE:
						::syslog( LOG_NOTICE, "%s", logger.str().c_str() );
						break;

					case LOGGER_INFO:
						::syslog( LOG_INFO, "%s", logger.str().c_str() );
						break;

					case LOGGER_DEBUG:
						::syslog( LOG_DEBUG, "%s", logger.str().c_str() );
						break;

					default:
						::syslog( LOG_NOTICE, "%s", logger.str().c_str() );
						break;
					}
#endif
				}
			}
#endif
		}
	}

	void Logger::LoggerOutput::log(const Logger &log)
	{
		if (_level & log._level)
		{
			std::list<std::string> prefixes;

			// check for prefixes
			if (_options & LOG_DATETIME)
			{
				// get timestamp
				std::string timestamp(asctime( localtime(&log._logtime.tv_sec) ));
				timestamp.erase(std::remove(timestamp.begin(), timestamp.end(), '\n'), timestamp.end());
				prefixes.push_back(timestamp);
			}

			// check for prefixes
			if (_options & LOG_TIMESTAMP)
			{
				std::stringstream ss;
				ss.fill('0');
				ss << log._logtime.tv_sec << "." << std::setw(6) << log._logtime.tv_usec;
				prefixes.push_back(ss.str());
			}

			if (_options & LOG_LEVEL)
			{
				// print log level
				switch (log._level)
				{
					case LOGGER_EMERG:
						prefixes.push_back("EMERGENCY");
						break;

					case LOGGER_ALERT:
						prefixes.push_back("ALERT");
						break;

					case LOGGER_CRIT:
						prefixes.push_back("CRTITICAL");
						break;

					case LOGGER_ERR:
						prefixes.push_back("ERROR");
						break;

					case LOGGER_WARNING:
						prefixes.push_back("WARNING");
						break;

					case LOGGER_NOTICE:
						prefixes.push_back("NOTICE");
						break;

					case LOGGER_INFO:
						prefixes.push_back("INFO");
						break;

					case LOGGER_DEBUG:
					{
						std::stringstream ss;
						ss << "DEBUG." << log._debug_verbosity;
						prefixes.push_back(ss.str());
						break;
					}

					default:
						break;
				}
			}

			if (_options & LOG_HOSTNAME)
			{
				std::vector<char> hostname_array(64);
				if ( gethostname(&hostname_array[0], 64) == 0 )
				{
					std::string hostname(&hostname_array[0]);
					prefixes.push_back(hostname);
				}
			}

			if (_options & LOG_TAG)
			{
				if (log._tag.length() > 0) {
					prefixes.push_back(log._tag);
				} else {
					prefixes.push_back(Logger::_default_tag);
				}
			}

			// print prefixes
			for (std::list<std::string>::const_iterator iter = prefixes.begin(); iter != prefixes.end(); iter++)
			{
				if (iter == prefixes.begin())
				{
					_stream << (*iter);
				}
				else
				{
					_stream << " " << (*iter);
				}
			}

			if (!prefixes.empty())
			{
				_stream << ": ";
			}

			_stream << log.str() << std::endl;
		}
	}

	Logger::LoggerOutput::LoggerOutput(std::ostream &stream, const unsigned char logmask, const unsigned char options)
	 : _stream(stream), _level(logmask), _options(options)
	{
	}

	Logger::LoggerOutput::~LoggerOutput()
	{
	}

	void Logger::enableAsync()
	{
		Logger::_logwriter.enableAsync();
	}

	void Logger::enableBuffer(size_t size)
	{
		Logger::_logwriter.enableBuffer(size);
	}

	void Logger::writeBuffer(std::ostream &stream)
	{
		unsigned char logmask = ibrcommon::Logger::LOGGER_ALL;
		unsigned char options = ibrcommon::Logger::LOG_DATETIME | ibrcommon::Logger::LOG_LEVEL;

		Logger::_logwriter.writeBuffer(stream, logmask, options);
	}

	void Logger::LogWriter::enableAsync()
	{
		try {
			_use_queue = true;
			start();
		} catch (const ibrcommon::ThreadException &ex) {
			IBRCOMMON_LOGGER(error) << "failed to start LogWriter\n" << ex.what() << IBRCOMMON_LOGGER_ENDL;
		}
	}
	
	void Logger::setDefaultTag(const std::string &tag)
	{
		Logger::_default_tag = tag;
	}

	void Logger::stop()
	{
		// stop the LogWriter::run() thread (this is needed with uclibc)
		_logwriter.stop();
	}

	void Logger::reload()
	{
		//reload the logger
		_logwriter.reload();
	}

	Logger::LogWriter::LogWriter()
	 : _global_logmask(0), _verbosity(0), _syslog(0), _syslog_mask(0), _queue(50), _use_queue(false), _buffer_size(0), _buffer(NULL),
	   _logfile_output(NULL), _logfile_logmask(0), _logfile_options(0)
	// limit queue to 50 entries
	{
	}

	Logger::LogWriter::~LogWriter()
	{
		// do cleanup only if the thread was running before
		if (_use_queue) stop();

		// join the LogWriter::run() thread
		join();

		// remove the ring-buffer
		ibrcommon::MutexLock l(_buffer_mutex);
		if (_buffer != NULL) delete _buffer;
	}

	void Logger::LogWriter::enableBuffer(size_t size)
	{
		ibrcommon::MutexLock l(_buffer_mutex);
		_buffer_size = size;
		_buffer = new std::list<Logger>();
	}

	void Logger::LogWriter::setLogfile(const ibrcommon::File &logfile, const unsigned char logmask, const unsigned char options)
	{
		ibrcommon::MutexLock l(_logfile_mutex);
		if (_logfile_output != NULL)
		{
			// delete the old logger
			delete _logfile_output;
			_logfile_output = NULL;

			// close the output file
			_logfile_stream.close();
		}

		_logfile = logfile;
		_logfile_logmask = logmask;
		_logfile_options = options;
		_global_logmask |= logmask;

		// open the new logfile
		_logfile_stream.open(_logfile.getPath().c_str(), std::ios::out | std::ios::app);

		// create a new logger output
		if (_logfile_stream.good())
		{
			_logfile_output = new LoggerOutput(_logfile_stream, _logfile_logmask, _logfile_options);
		}
		else
		{
			_logfile_stream.close();
		}
	}

	void Logger::LogWriter::reload()
	{
		ibrcommon::MutexLock l(_logfile_mutex);
		if (_logfile_output != NULL)
		{
			// delete the old logger
			delete _logfile_output;
			_logfile_output = NULL;

			// close the output file
			_logfile_stream.close();

			// open the new logfile
			_logfile_stream.open(_logfile.getPath().c_str(), std::ios::out | std::ios::app);

			// create a new logger output
			if (_logfile_stream.good())
			{
				_logfile_output = new LoggerOutput(_logfile_stream, _logfile_logmask, _logfile_options);
			}
			else
			{
				_logfile_stream.close();
			}
		}
	}

	void Logger::LogWriter::writeBuffer(std::ostream &stream, const unsigned char logmask, const unsigned char options)
	{
		ibrcommon::MutexLock l(_buffer_mutex);
		if (_buffer == NULL) return;

		LoggerOutput output(stream, logmask, options);

		for (std::list<Logger>::const_iterator iter = _buffer->begin(); iter != _buffer->end(); iter++)
		{
			const Logger &l = (*iter);
			output.log(l);
		}
	}

	void Logger::LogWriter::log(Logger &logger)
	{
		if (_use_queue)
		{
			_queue.push(logger);
		}
		else
		{
			flush(logger);
		}
	}

	void Logger::LogWriter::run() throw ()
	{
		try {
			while (true)
			{
				Logger log = _queue.getnpop(true);
				flush(log);

				// add to ring-buffer
				ibrcommon::MutexLock l(_buffer_mutex);
				if (_buffer != NULL)
				{
					_buffer->push_back(log);
					while (_buffer->size() > _buffer_size)
					{
						_buffer->pop_front();
					}
				}
			}
		} catch (const std::exception&) {
			ibrcommon::Queue<Logger>::Locked q = _queue.exclusive();

			try {
				// In this block we will write all remaining element in the queue
				// to the logging streams. While we do this, the queue should be locked
				// and finally we abort the queue to unblock all waiting threads.
				while (!q.empty())
				{
					Logger &log = q.front();
					try { flush(log); } catch (const std::exception&) {};

					q.pop();
				}
			} catch (const std::exception&) {

			}
		}
	}

	void Logger::LogWriter::__cancellation() throw ()
	{
		// cancel the main thread in here
		_queue.abort();
	}
}
