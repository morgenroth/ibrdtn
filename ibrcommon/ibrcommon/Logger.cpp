/*
 * Logger.cpp
 *
 *  Created on: 08.06.2010
 *      Author: morgenro
 */

#include "ibrcommon/Logger.h"
#include "ibrcommon/SyslogStream.h"
#include <algorithm>
#include <sys/time.h>
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/MutexLock.h>
#include <iomanip>


namespace ibrcommon
{
	Logger::LogWriter Logger::_logwriter;

	Logger::Logger(LogLevel level, int debug_verbosity)
	 : _level(level), _debug_verbosity(debug_verbosity)
	{
		::gettimeofday(&_logtime, NULL);
	}

	Logger::Logger(const Logger &obj)
	 : std::stringstream(obj.str()), _level(obj._level), _debug_verbosity(obj._debug_verbosity), _logtime(obj._logtime)
	{
	}

	Logger::~Logger()
	{
	}

	Logger Logger::emergency()
	{
		return Logger(LOGGER_EMERG);
	}

	Logger Logger::alert()
	{
		return Logger(LOGGER_ALERT);
	}

	Logger Logger::critical()
	{
		return Logger(LOGGER_CRIT);
	}

	Logger Logger::error()
	{
		return Logger(LOGGER_ERR);
	}

	Logger Logger::warning()
	{
		return Logger(LOGGER_WARNING);
	}

	Logger Logger::notice()
	{
		return Logger(LOGGER_NOTICE);
	}

	Logger Logger::info()
	{
		return Logger(LOGGER_INFO);
	}

	Logger Logger::debug(int verbosity)
	{
		return Logger(LOGGER_DEBUG, verbosity);
	}

	void Logger::setVerbosity(const int verbosity)
	{
		Logger::_logwriter.setVerbosity(verbosity);
	}

	void Logger::LogWriter::setVerbosity(const int verbosity)
	{
		_verbosity = verbosity;
	}

	int Logger::getVerbosity()
	{
		return Logger::_logwriter.getVerbosity();
	}

	int Logger::LogWriter::getVerbosity()
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
		// init syslog
		::openlog(name, option, facility);
		_syslog = true;
		_syslog_mask = logmask;
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

			// additionally log to the syslog
			if (_syslog)
			{
				if (logger._level & _syslog_mask)
				{
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
				}
			}
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
				char *hostname_array = new char[64];
				if ( gethostname(hostname_array, 64) == 0 )
				{
					std::string hostname(hostname_array);
					prefixes.push_back(hostname);
				}

				delete[] hostname_array;
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
	 : _verbosity(0), _syslog(0), _syslog_mask(0), _queue(50), _use_queue(false), _buffer_size(0), _buffer(NULL),
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

	void Logger::LogWriter::run()
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
					try { log.flush(); } catch (const std::exception&) {};

					q.pop();
				}
			} catch (const std::exception&) {

			}
		}
	}

	void Logger::LogWriter::__cancellation()
	{
		// cancel the main thread in here
		_queue.abort();
	}
}
