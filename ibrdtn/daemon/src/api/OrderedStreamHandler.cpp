/*
 * OrderedStreamHandler.cpp
 *
 *  Created on: 17.11.2011
 *      Author: morgenro
 */

#include "api/OrderedStreamHandler.h"
#include "core/BundleCore.h"
#include "net/BundleReceivedEvent.h"

#include <ibrdtn/data/PrimaryBlock.h>
#include <ibrdtn/utils/Utils.h>
#include <ibrcommon/Logger.h>

#ifdef WITH_COMPRESSION
#include <ibrdtn/data/CompressedPayloadBlock.h>
#endif

#ifdef WITH_BUNDLE_SECURITY
#include "security/SecurityManager.h"
#endif

namespace dtn
{
	namespace api
	{
		OrderedStreamHandler::OrderedStreamHandler(ClientHandler &client, ibrcommon::tcpstream &stream)
		 : ProtocolHandler(client, stream), _sender(*this), _streambuf(*this), _bundlestream(&_streambuf), _group(true), _lifetime(3600)
		{
			_endpoint = dtn::core::BundleCore::local + "/" + client.getRegistration().getHandle();
		}

		OrderedStreamHandler::~OrderedStreamHandler()
		{
			_sender.stop();
			_sender.join();
		}

		void OrderedStreamHandler::delivered(const dtn::data::MetaBundle &m)
		{
			_client.getRegistration().delivered(m);
		}

		void OrderedStreamHandler::put(dtn::data::Bundle &b)
		{
			IBRCOMMON_LOGGER_DEBUG(20) << "OrderedStreamHandler: put()" << IBRCOMMON_LOGGER_ENDL;

			// set destination EID
			b._destination = _peer;

			// set source
			b._source = _endpoint;

			// set lifetime
			b._lifetime = _lifetime;

			// set flag if the bundles are addresses to a group
			if (_group)
			{
				b.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, false);
			}
			else
			{
				b.set(dtn::data::PrimaryBlock::DESTINATION_IS_SINGLETON, true);
			}

			// raise default bundle received event
			dtn::net::BundleReceivedEvent::raise(dtn::core::BundleCore::local + _client.getRegistration().getHandle(), b, true, true);
		}

		dtn::data::MetaBundle OrderedStreamHandler::get(size_t timeout)
		{
			Registration &reg = _client.getRegistration();
			IBRCOMMON_LOGGER_DEBUG(20) << "OrderedStreamHandler: get()" << IBRCOMMON_LOGGER_ENDL;

			while (true)
			{
				try {
					dtn::data::MetaBundle bundle = reg.receiveMetaBundle();

					// discard bundle if they are not from the specified peer
					if ((!_group) && (bundle.source != _peer))
					{
						IBRCOMMON_LOGGER_DEBUG(30) << "OrderedStreamHandler: get(): bundle source " << bundle.source.getString() << " not expected - discard" << IBRCOMMON_LOGGER_ENDL;
						continue;
					}

					return bundle;
				} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) {
					IBRCOMMON_LOGGER_DEBUG(30) << "OrderedStreamHandler: get(): no bundle found wait for notify" << IBRCOMMON_LOGGER_ENDL;
					reg.wait_for_bundle(timeout);
				}
			}
		}

		void OrderedStreamHandler::__cancellation()
		{
			// close the stream
			try {
				_stream.close();
			} catch (const ibrcommon::ConnectionClosedException&) { };
		}

		void OrderedStreamHandler::finally()
		{
			IBRCOMMON_LOGGER_DEBUG(60) << "OrderedStreamHandler down" << IBRCOMMON_LOGGER_ENDL;

			_client.getRegistration().abort();

			// close the stream
			try {
				_stream.close();
			} catch (const ibrcommon::ConnectionClosedException&) { };

			try {
				// shutdown the sender thread
				_sender.stop();
			} catch (const std::exception&) { };
		}

		void OrderedStreamHandler::run()
		{
			std::string buffer;
			_stream << ClientHandler::API_STATUS_OK << " SWITCHED TO ORDERED STREAM PROTOCOL" << std::endl;

			while (_stream.good())
			{
				getline(_stream, buffer);

				std::string::reverse_iterator iter = buffer.rbegin();
				if ( (*iter) == '\r' ) buffer = buffer.substr(0, buffer.length() - 1);

				std::vector<std::string> cmd = dtn::utils::Utils::tokenize(" ", buffer);
				if (cmd.size() == 0) continue;

				try {
					if (cmd[0] == "connect")
					{
						_stream << ClientHandler::API_STATUS_CONTINUE << " CONNECTION ESTABLISHED" << std::endl;

						// start sender to transfer received payload to the client
						_sender.start();

						// forward data to stream buffer
						_bundlestream << _stream.rdbuf() << std::flush;
					}
					else if (cmd[0] == "set")
					{
						if (cmd.size() < 2) throw ibrcommon::Exception("not enough parameters");

						if (cmd[1] == "endpoint")
						{
							if (cmd.size() < 3) throw ibrcommon::Exception("not enough parameters");

							_endpoint = dtn::core::BundleCore::local + "/" + cmd[2];

							// error checking
							if (_endpoint == dtn::data::EID())
							{
								_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " INVALID ENDPOINT" << std::endl;
								_endpoint = dtn::core::BundleCore::local;
							}
							else
							{
								_client.getRegistration().subscribe(_endpoint);
								_stream << ClientHandler::API_STATUS_OK << " OK" << std::endl;
							}
						}
						else if (cmd[1] == "destination")
						{
							_peer = cmd[2];
							_group = false;
							_stream << ClientHandler::API_STATUS_OK << " DESTINATION CHANGED" << std::endl;
						}
						else if (cmd[1] == "group")
						{
							_peer = cmd[2];
							_group = true;
							_stream << ClientHandler::API_STATUS_OK << " DESTINATION GROUP CHANGED" << std::endl;
						}
						else if (cmd[1] == "lifetime")
						{
							std::stringstream ss(cmd[2]);
							ss >> _lifetime;
							_stream << ClientHandler::API_STATUS_OK << " LIFETIME CHANGED" << std::endl;
						}
						else if (cmd[1] == "chunksize")
						{
							size_t size = 0;
							std::stringstream ss(cmd[2]);
							ss >> size;
							_streambuf.setChunkSize(size);
							_stream << ClientHandler::API_STATUS_OK << " CHUNKSIZE CHANGED" << std::endl;
						}
						else if (cmd[1] == "timeout")
						{
							size_t timeout = 0;
							std::stringstream ss(cmd[2]);
							ss >> timeout;
							_streambuf.setTimeout(timeout);
							_stream << ClientHandler::API_STATUS_OK << " TIMEOUT CHANGED" << std::endl;
						}
						else
						{
							_stream << ClientHandler::API_STATUS_BAD_REQUEST << " UNKNOWN COMMAND" << std::endl;
						}
					}
					else
					{
						_stream << ClientHandler::API_STATUS_BAD_REQUEST << " UNKNOWN COMMAND" << std::endl;
					}
				} catch (const std::exception&) {
					_stream << ClientHandler::API_STATUS_BAD_REQUEST << " ERROR" << std::endl;
				}
			}
		}

		OrderedStreamHandler::Sender::Sender(OrderedStreamHandler &conn)
		 : _handler(conn)
		{
		}

		OrderedStreamHandler::Sender::~Sender()
		{
			ibrcommon::JoinableThread::join();
		}

		void OrderedStreamHandler::Sender::__cancellation()
		{
			// cancel the main thread in here
			_handler._client.getRegistration().abort();
		}

		void OrderedStreamHandler::Sender::finally()
		{
			_handler._client.getRegistration().abort();
		}

		void OrderedStreamHandler::Sender::run()
		{
			try {
				_handler._stream << _handler._bundlestream.rdbuf() << std::flush;
			} catch (const std::exception &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "unexpected API error! " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}
		}
	} /* namespace api */
} /* namespace dtn */
