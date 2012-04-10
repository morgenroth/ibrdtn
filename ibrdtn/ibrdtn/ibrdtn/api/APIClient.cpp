/*
 * APIClient.cpp
 *
 *  Created on: 01.07.2011
 *      Author: morgenro
 */

#include "ibrdtn/api/APIClient.h"
#include "ibrdtn/data/Bundle.h"
#include "ibrdtn/api/PlainSerializer.h"
#include "ibrdtn/utils/Utils.h"
#include <ibrcommon/Logger.h>

#include <iostream>

namespace dtn
{
	namespace api
	{
		APIClient::Message::Message(STATUS_CODES c, const std::string &m)
		 : code(c), msg(m)
		{
		}

		APIClient::Message::~Message()
		{}

		APIClient::APIClient(ibrcommon::tcpstream &stream)
		  : _stream(stream), _get_busy(false)
		{
		}

		APIClient::~APIClient()
		{
		}

		void APIClient::connect()
		{
			// receive connection banner
			std::string banner;
			getline(_stream, banner);
		}

		APIClient::Message APIClient::__get_message()
		{
			std::string buffer;
			std::stringstream ss;
			int code = 0;

			if (_stream.eof()) throw ibrcommon::Exception("connection closed");

			getline(_stream, buffer);
			ss.clear();
			ss.str(buffer);
			ss >> code;

			return Message(APIClient::STATUS_CODES(code), buffer);
		}

		int APIClient::__get_return()
		{
			try {
				ibrcommon::MutexLock l(_queue_cond);

				while (_status_queue.empty())
				{
					if (_get_busy)
					{
						_queue_cond.wait();
					}
					else
					{
						_get_busy = true;
						throw true;
					}
				}

				// queue has an element... get it
				Message m = _status_queue.front();
				_status_queue.pop();

				return m.code;
			} catch (bool) {
				while (true)
				{
					Message m = __get_message();

					ibrcommon::MutexLock l(_queue_cond);
					if ((m.code >= 600) && (m.code < 700))
					{
						_notify_queue.push(m);
						_queue_cond.signal(true);
					}
					else
					{
						_get_busy = false;
						_queue_cond.signal(true);
						return m.code;
					}
				}
			}
		}

		void APIClient::setEndpoint(const std::string &endpoint)
		{
			_stream << "set endpoint " << endpoint << std::endl;
			if (__get_return() != API_STATUS_ACCEPTED)
			{
				// error
				throw ibrcommon::Exception("endpoint invalid");
			}
		}

		void APIClient::subscribe(const dtn::data::EID &eid)
		{
			_stream << "registration add " << eid.getString() << std::endl;
			if (__get_return() != API_STATUS_ACCEPTED)
			{
				// error
				throw ibrcommon::Exception("eid invalid");
			}
		}

		void APIClient::unsubscribe(const dtn::data::EID &eid)
		{
			_stream << "registration del " << eid.getString() << std::endl;
			if (__get_return() != API_STATUS_ACCEPTED)
			{
				// error
				throw ibrcommon::Exception("eid invalid");
			}
		}

		std::list<dtn::data::EID> APIClient::getSubscriptions()
		{
			std::list<dtn::data::EID> sublst;
			_stream << "registration list" << std::endl;

			if (__get_return() == API_STATUS_OK)
			{
				std::string buffer;

				while (!_stream.eof())
				{
					getline(_stream, buffer);

					// end of list?
					if (buffer.size() == 0) break;

					// add eid to the list
					sublst.push_back(buffer);
				}
			}
			else
			{
				// error
				throw ibrcommon::Exception("get failed");
			}

			return sublst;
		}

		dtn::api::Bundle APIClient::get(dtn::data::BundleID &id)
		{
			_stream << "bundle load " << id.timestamp << " " << id.sequencenumber << " ";

			if (id.fragment)
			{
				_stream << id.offset << " ";
			}

			_stream << id.source.getString() << std::endl;

			switch (__get_return())
			{
			case API_STATUS_NOT_FOUND:
				throw ibrcommon::Exception("bundle not found");

			case API_STATUS_OK:
				break;

			default:
				throw ibrcommon::Exception("load error");
			}

			return register_get();
		}

		dtn::api::Bundle APIClient::get()
		{
			_stream << "bundle load queue" << std::endl;

			switch (__get_return())
			{
			case API_STATUS_NOT_FOUND:
				throw ibrcommon::Exception("bundle not found");

			case API_STATUS_OK:
				break;

			default:
				throw ibrcommon::Exception("load error");
			}

			return register_get();
		}

		void APIClient::send(const dtn::api::Bundle &bundle)
		{
			register_put(bundle._b);
			register_send();
		}

		void APIClient::notify_delivered(const dtn::api::Bundle &b)
		{
			dtn::data::BundleID id(b._b);
			notify_delivered(id);
		}

		void APIClient::notify_delivered(const dtn::data::BundleID &id)
		{
			_stream << "bundle delivered " << id.timestamp << " " << id.sequencenumber << " ";

			if (id.fragment)
			{
				_stream << id.offset << " ";
			}

			_stream << id.source.getString() << std::endl;

			switch (__get_return())
			{
			case API_STATUS_NOT_FOUND:
				throw ibrcommon::Exception("bundle not found");

			case API_STATUS_OK:
				break;

			default:
				throw ibrcommon::Exception("notify delivered error");
			}
		}

		APIClient::Message APIClient::wait()
		{
			try {
				ibrcommon::MutexLock l(_queue_cond);

				while (_notify_queue.empty())
				{
					if (_get_busy)
					{
						_queue_cond.wait();
					}
					else
					{
						_get_busy = true;
						throw true;
					}
				}

				// queue has an element... get it
				Message m = _notify_queue.front();
				_notify_queue.pop();

				return m;
			} catch (bool) {
				while (true)
				{
					Message m = __get_message();

					ibrcommon::MutexLock l(_queue_cond);
					if ((m.code < 600) || (m.code >= 700))
					{
						_status_queue.push(m);
						_queue_cond.signal(true);
					}
					else
					{
						_get_busy = false;
						_queue_cond.signal(true);
						return m;
					}
				}
			}
		}

		void APIClient::unblock_wait()
		{
			ibrcommon::MutexLock l(_queue_cond);
			_queue_cond.abort();
		}

		void APIClient::register_put(const dtn::data::Bundle &bundle)
		{
			_stream << "bundle put plain" << std::endl;

			switch (__get_return())
			{
			case API_STATUS_NOT_ACCEPTABLE:
				throw ibrcommon::Exception("not acceptable");

			case API_STATUS_CONTINUE:
				break;

			default:
				throw ibrcommon::Exception("put error");
			}

			dtn::api::PlainSerializer(_stream) << bundle; _stream << std::flush;

			switch (__get_return())
			{
			case API_STATUS_NOT_ACCEPTABLE:
				throw ibrcommon::Exception("not acceptable");

			case API_STATUS_OK:
				break;

			default:
				throw ibrcommon::Exception("put error");
			}
		}

		void APIClient::register_clear()
		{
			_stream << "bundle clear" << std::endl;

			switch (__get_return())
			{
			case API_STATUS_OK:
				break;

			default:
				throw ibrcommon::Exception("clear error");
			}
		}

		void APIClient::register_free()
		{
			_stream << "bundle free" << std::endl;

			switch (__get_return())
			{
			case API_STATUS_OK:
				break;

			default:
				throw ibrcommon::Exception("free error");
			}
		}

		void APIClient::register_store()
		{
			_stream << "bundle store" << std::endl;

			switch (__get_return())
			{
			case API_STATUS_OK:
				break;

			default:
				throw ibrcommon::Exception("store error");
			}
		}

		void APIClient::register_send()
		{
			_stream << "bundle send" << std::endl;

			switch (__get_return())
			{
			case API_STATUS_OK:
				break;

			default:
				throw ibrcommon::Exception("send error");
			}
		}

		dtn::data::Bundle APIClient::register_get()
		{
			dtn::data::Bundle ret;
			_stream << "bundle get plain" << std::endl;

			switch (__get_return())
			{
			case API_STATUS_OK:
				dtn::api::PlainDeserializer(_stream) >> ret;
				break;

			default:
				throw ibrcommon::Exception("get error");
			}

			return ret;
		}

		dtn::data::BundleID APIClient::readBundleID(const std::string &data)
		{
			std::vector<std::string> vdata = dtn::utils::Utils::tokenize(" ", data);

			// load bundle id
			std::stringstream ss;
			size_t timestamp = 0;
			size_t sequencenumber = 0;
			bool fragment = false;
			size_t offset = 0;

			// read timestamp
			 ss.clear(); ss.str(vdata[0]); ss >> timestamp;

			// read sequence number
			 ss.clear(); ss.str(vdata[1]); ss >> sequencenumber;

			// read fragment offset
			if (vdata.size() > 3)
			{
				fragment = true;

				// read sequence number
				 ss.clear(); ss.str(vdata[2]); ss >> offset;
			}

			// read EID
			 ss.clear(); dtn::data::EID eid(vdata[vdata.size() - 1]);

			// construct bundle id
			return dtn::data::BundleID(eid, timestamp, sequencenumber, fragment, offset);
		}
	}
}
