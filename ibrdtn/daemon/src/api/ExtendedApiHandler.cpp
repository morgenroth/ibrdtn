/*
 * ExtendedApiHandler.cpp
 *
 * Copyright (C) 2011 IBR, TU Braunschweig
 *
 * Written-by: Johannes Morgenroth <morgenroth@ibr.cs.tu-bs.de>
 * Written-by: Stephen Roettger <roettger@ibr.cs.tu-bs.de>
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
#include "Configuration.h"
#include "api/ExtendedApiHandler.h"
#include "core/BundleEvent.h"
#include <ibrcommon/Logger.h>
#include <ibrdtn/api/PlainSerializer.h>
#include <ibrdtn/data/AgeBlock.h>
#include <ibrdtn/utils/Utils.h>
#include <ibrcommon/data/Base64Reader.h>
#include "core/BundleCore.h"
#include <ibrdtn/utils/Random.h>

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
		ExtendedApiHandler::ExtendedApiHandler(ClientHandler &client, ibrcommon::socketstream &stream)
		 : ProtocolHandler(client, stream), _sender(new Sender(*this)),
		   _endpoint(_client.getRegistration().getDefaultEID())
		{
			_client.getRegistration().subscribe(_endpoint);
		}

		ExtendedApiHandler::~ExtendedApiHandler()
		{
			_client.getRegistration().abort();
			_sender->join();
		}

		bool ExtendedApiHandler::good() const{
			return _stream.good();
		}

		void ExtendedApiHandler::__cancellation() throw ()
		{
			// close the stream
			_stream.close();
		}

		void ExtendedApiHandler::finally()
		{
			IBRCOMMON_LOGGER_DEBUG(60) << "ExtendedApiConnection down" << IBRCOMMON_LOGGER_ENDL;

			_client.getRegistration().abort();

			// close the stream
			_stream.close();

			try {
				// shutdown the sender thread
				_sender->stop();
			} catch (const std::exception&) { };
		}

		void ExtendedApiHandler::run()
		{
			_sender->start();

			std::string buffer;
			_stream << ClientHandler::API_STATUS_OK << " SWITCHED TO EXTENDED" << std::endl;

			while (_stream.good())
			{
				getline(_stream, buffer);

				std::string::reverse_iterator iter = buffer.rbegin();
				if ( (*iter) == '\r' ) buffer = buffer.substr(0, buffer.length() - 1);

				std::vector<std::string> cmd = dtn::utils::Utils::tokenize(" ", buffer);
				if (cmd.size() == 0) continue;

				try {
					if (cmd[0] == "set")
					{
						if (cmd.size() < 2) throw ibrcommon::Exception("not enough parameters");

						if (cmd[1] == "endpoint")
						{
							if (cmd.size() < 3) throw ibrcommon::Exception("not enough parameters");

							ibrcommon::MutexLock l(_write_lock);
							dtn::data::EID new_endpoint = dtn::core::BundleCore::local + "/" + cmd[2];

							// error checking
							if (new_endpoint == dtn::data::EID())
							{
								_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " INVALID ENDPOINT" << std::endl;
							}
							else
							{
								/* unsubscribe from the old endpoint and subscribe to the new one */
								Registration& reg = _client.getRegistration();
								reg.unsubscribe(_endpoint);
								reg.subscribe(new_endpoint);
								_endpoint = new_endpoint;
								_stream << ClientHandler::API_STATUS_OK << " OK" << std::endl;
							}
						}
						else
						{
							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_BAD_REQUEST << " UNKNOWN COMMAND" << std::endl;
						}
					}
					else if (cmd[0] == "endpoint")
					{
						if (cmd.size() < 2) throw ibrcommon::Exception("not enough parameters");

						if (cmd[1] == "add")
						{
							if (cmd.size() < 3) throw ibrcommon::Exception("not enough parameters");

							ibrcommon::MutexLock l(_write_lock);
							dtn::data::EID new_endpoint = dtn::core::BundleCore::local + "/" + cmd[2];

							// error checking
							if (new_endpoint == dtn::data::EID())
							{
								_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " INVALID ENDPOINT" << std::endl;
							}
							else
							{
								_client.getRegistration().subscribe(new_endpoint);
								_stream << ClientHandler::API_STATUS_OK << " OK" << std::endl;
							}
						}
						else if (cmd[1] == "del")
						{
							dtn::data::EID del_endpoint = _endpoint;
							if (cmd.size() >= 3)
							{
								del_endpoint = dtn::core::BundleCore::local + "/" + cmd[2];
							}

							ibrcommon::MutexLock l(_write_lock);

							// error checking
							if (del_endpoint == dtn::data::EID())
							{
								_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " INVALID ENDPOINT" << std::endl;
							}
							else
							{
								_client.getRegistration().unsubscribe(del_endpoint);

								if(_endpoint == del_endpoint)
								{
									_endpoint = _client.getRegistration().getDefaultEID();
									_client.getRegistration().subscribe(_endpoint);
								}

								_stream << ClientHandler::API_STATUS_OK << " OK" << std::endl;
							}
						}
						else if (cmd[1] == "get")
						{
							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_OK << " ENDPOINT GET " << _endpoint.getString() << std::endl;
						}
						else
						{
							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_BAD_REQUEST << " UNKNOWN COMMAND" << std::endl;
						}
					}
					else if (cmd[0] == "registration")
					{
						if (cmd.size() < 2) throw ibrcommon::Exception("not enough parameters");

						if (cmd[1] == "add")
						{
							if (cmd.size() < 3) throw ibrcommon::Exception("not enough parameters");

							ibrcommon::MutexLock l(_write_lock);
							dtn::data::EID endpoint(cmd[2]);

							// error checking
							if (endpoint == dtn::data::EID())
							{
								_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " INVALID EID" << std::endl;
							}
							else
							{
								_client.getRegistration().subscribe(endpoint);
								_stream << ClientHandler::API_STATUS_OK << " OK" << std::endl;
							}
						}
						else if (cmd[1] == "del")
						{
							if (cmd.size() < 3) throw ibrcommon::Exception("not enough parameters");

							ibrcommon::MutexLock l(_write_lock);
							dtn::data::EID endpoint(cmd[2]);

							// error checking
							if (endpoint == dtn::data::EID())
							{
								_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " INVALID EID" << std::endl;
							}
							else
							{
								_client.getRegistration().unsubscribe(endpoint);
								if(_endpoint == endpoint)
								{
									_endpoint = _client.getRegistration().getDefaultEID();
									_client.getRegistration().subscribe(_endpoint);
								}

								_stream << ClientHandler::API_STATUS_OK << " OK" << std::endl;
							}
						}
						else if (cmd[1] == "list")
						{
							ibrcommon::MutexLock l(_write_lock);
							const std::set<dtn::data::EID> list = _client.getRegistration().getSubscriptions();

							_stream << ClientHandler::API_STATUS_OK << " REGISTRATION LIST" << std::endl;
							for (std::set<dtn::data::EID>::const_iterator iter = list.begin(); iter != list.end(); iter++)
							{
								_stream << (*iter).getString() << std::endl;
							}
							_stream << std::endl;
						}
						else if (cmd[1] == "save")
						{
							if (cmd.size() < 3) throw ibrcommon::Exception("not enough parameters");

							size_t lifetime = 0;
							std::stringstream ss(cmd[2]);

							ss >> lifetime;
							if(ss.fail()) throw ibrcommon::Exception("malformed command");

							/* make the registration persistent for a given lifetime */
							_client.getRegistration().setPersistent(lifetime);

							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_OK << " REGISTRATION SAVE " << _client.getRegistration().getHandle() << std::endl;
						}
						else if (cmd[1] == "load")
						{
							if (cmd.size() < 3) throw ibrcommon::Exception("not enough parameters");

							const std::string handle = cmd[2];

							try
							{
								Registration& reg = _client.getAPIServer().getRegistration(handle);

								/* stop the sender */
								_client.getRegistration().abort();
								_sender->join();

								/* switch the registration */
								_client.switchRegistration(reg);

								/* and switch the sender */
								Sender *old_sender = _sender;
								try{
									_sender = new Sender(*this);
								}
								catch (const std::bad_alloc &ex)
								{
									_sender = old_sender;
									throw ex;
								}
								delete old_sender;
								_sender->start();

								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_OK << " REGISTRATION LOAD" << std::endl;
							}
							catch (const Registration::AlreadyAttachedException& ex)
							{
								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_SERVICE_UNAVAILABLE << " REGISTRATION BUSY" << std::endl;
							}
							catch (const Registration::NotFoundException& ex)
							{
								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_SERVICE_UNAVAILABLE << " REGISTRATION NOT FOUND" << std::endl;
							}
						}
						else
						{
							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_BAD_REQUEST << " UNKNOWN COMMAND" << std::endl;
						}
					}
					else if (cmd[0] == "neighbor")
					{
						if (cmd.size() < 2) throw ibrcommon::Exception("not enough parameters");

						if (cmd[1] == "list")
						{
							ibrcommon::MutexLock l(_write_lock);
							const std::set<dtn::core::Node> nlist = dtn::core::BundleCore::getInstance().getConnectionManager().getNeighbors();

							_stream << ClientHandler::API_STATUS_OK << " NEIGHBOR LIST" << std::endl;
							for (std::set<dtn::core::Node>::const_iterator iter = nlist.begin(); iter != nlist.end(); iter++)
							{
								_stream << (*iter).getEID().getString() << std::endl;
							}
							_stream << std::endl;
						}
						else
						{
							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_BAD_REQUEST << " UNKNOWN COMMAND" << std::endl;
						}
					}
					else if (cmd[0] == "bundle")
					{
						if (cmd.size() < 2) throw ibrcommon::Exception("not enough parameters");

						if (cmd[1] == "get")
						{
							// transfer bundle data
							ibrcommon::MutexLock l(_write_lock);

							if (cmd.size() == 2)
							{
								_stream << ClientHandler::API_STATUS_OK << " BUNDLE GET "; sayBundleID(_stream, _bundle_reg); _stream << std::endl;
								PlainSerializer(_stream) << _bundle_reg;
							}
							else if (cmd[2] == "binary")
							{
								_stream << ClientHandler::API_STATUS_OK << " BUNDLE GET BINARY "; sayBundleID(_stream, _bundle_reg); _stream << std::endl;
								dtn::data::DefaultSerializer(_stream) << _bundle_reg; _stream << std::flush;
							}
							else if (cmd[2] == "plain")
							{
								_stream << ClientHandler::API_STATUS_OK << " BUNDLE GET PLAIN "; sayBundleID(_stream, _bundle_reg); _stream << std::endl;
								PlainSerializer(_stream) << _bundle_reg;
							}
							else if (cmd[2] == "xml")
							{
								_stream << ClientHandler::API_STATUS_NOT_IMPLEMENTED << " FORMAT NOT IMPLEMENTED" << std::endl;
							}
							else
							{
								_stream << ClientHandler::API_STATUS_BAD_REQUEST << " UNKNOWN FORMAT" << std::endl;
							}
						}
						else if (cmd[1] == "put")
						{
							// lock the stream during reception of bundle data
							ibrcommon::MutexLock l(_write_lock);

							if (cmd.size() < 3)
							{
								_stream << ClientHandler::API_STATUS_BAD_REQUEST << " PLEASE DEFINE THE FORMAT" << std::endl;
							}
							else if (cmd[2] == "plain")
							{
								_stream << ClientHandler::API_STATUS_CONTINUE << " PUT BUNDLE PLAIN" << std::endl;

								try {
									PlainDeserializer(_stream) >> _bundle_reg;
									_stream << ClientHandler::API_STATUS_OK << " BUNDLE IN REGISTER" << std::endl;
								} catch (const std::exception &ex) {
									IBRCOMMON_LOGGER_DEBUG(20) << "API put failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
									_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " PUT FAILED" << std::endl;

								}
							}
							else if (cmd[2] == "binary")
							{
								_stream << ClientHandler::API_STATUS_CONTINUE << " PUT BUNDLE BINARY" << std::endl;

								try {
									dtn::data::DefaultDeserializer(_stream) >> _bundle_reg;
									_stream << ClientHandler::API_STATUS_OK << " BUNDLE IN REGISTER" << std::endl;
								} catch (const std::exception &ex) {
									IBRCOMMON_LOGGER_DEBUG(20) << "API put failed: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
									_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " PUT FAILED" << std::endl;
								}
							}
							else
							{
								_stream << ClientHandler::API_STATUS_BAD_REQUEST << " PLEASE DEFINE THE FORMAT" << std::endl;
							}
						}
						else if (cmd[1] == "load")
						{
							if (cmd.size() < 3) throw ibrcommon::Exception("not enough parameters");

							dtn::data::BundleID id;

							if (cmd[2] == "queue")
							{
								id = _bundle_queue.getnpop();
							}
							else
							{
								// construct bundle id
								id = readBundleID(cmd, 2);
							}

							// load the bundle
							try {
								_bundle_reg = dtn::core::BundleCore::getInstance().getStorage().get(id);

								// process the bundle block (security, compression, ...)
								dtn::core::BundleCore::processBlocks(_bundle_reg);

								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_OK << " BUNDLE LOADED "; sayBundleID(_stream, id); _stream << std::endl;
							} catch (const ibrcommon::Exception&) {
								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_NOT_FOUND << " BUNDLE NOT FOUND" << std::endl;
							}
						}
						else if (cmd[1] == "clear")
						{
							_bundle_reg = dtn::data::Bundle();

							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_OK << " BUNDLE CLEARED" << std::endl;
						}
						else if (cmd[1] == "free")
						{
							try {
								dtn::core::BundleCore::getInstance().getStorage().remove(_bundle_reg);
								_bundle_reg = dtn::data::Bundle();
								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_OK << " BUNDLE FREE SUCCESSFUL" << std::endl;
							} catch (const ibrcommon::Exception&) {
								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_NOT_FOUND << " BUNDLE NOT FOUND" << std::endl;
							}
						}
						else if (cmd[1] == "delivered")
						{
							if (cmd.size() < 5) throw ibrcommon::Exception("not enough parameters");

							try {
								// construct bundle id
								dtn::data::BundleID id = readBundleID(cmd, 2);

								// announce this bundle as delivered
								dtn::data::MetaBundle meta = dtn::core::BundleCore::getInstance().getStorage().get(id);
								_client.getRegistration().delivered(meta);

								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_OK << " BUNDLE DELIVERED ACCEPTED" << std::endl;
							} catch (const ibrcommon::Exception&) {
								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_NOT_FOUND << " BUNDLE NOT FOUND" << std::endl;
							}
						}
						else if (cmd[1] == "store")
						{
							// store the bundle in the storage
							try {
								dtn::core::BundleCore::getInstance().getStorage().store(_bundle_reg);
								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_OK << " BUNDLE STORE SUCCESSFUL" << std::endl;
							} catch (const ibrcommon::Exception&) {
								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_INTERNAL_ERROR << " BUNDLE STORE FAILED" << std::endl;
							}
						}
						else if (cmd[1] == "send")
						{
							// create a new sequence number
							_bundle_reg.relabel();

							// forward the bundle to the storage processing
							_client.getAPIServer().processIncomingBundle(_endpoint, _bundle_reg);

							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_OK << " BUNDLE SENT" << std::endl;
						}
						else if (cmd[1] == "info")
						{
							// transfer bundle data
							ibrcommon::MutexLock l(_write_lock);

							_stream << ClientHandler::API_STATUS_OK << " BUNDLE INFO "; sayBundleID(_stream, _bundle_reg); _stream << std::endl;
							PlainSerializer(_stream, true) << _bundle_reg;
						}
						else if (cmd[1] == "block")
						{
							if (cmd.size() < 3) throw ibrcommon::Exception("not enough parameters");

							if (cmd[2] == "add")
							{
								PlainDeserializer::BlockInserter inserter(_bundle_reg, PlainDeserializer::BlockInserter::END);

								/* parse an optional offset, where to insert the block */
								if (cmd.size() > 3)
								{
									int offset;
									istringstream ss(cmd[3]);

									ss >> offset;
									if (ss.fail()) throw ibrcommon::Exception("malformed command");

									if (offset >= _bundle_reg.blockCount())
									{
										inserter = PlainDeserializer::BlockInserter(_bundle_reg, PlainDeserializer::BlockInserter::END);
									}
									else if(offset <= 0)
									{
										inserter = PlainDeserializer::BlockInserter(_bundle_reg, PlainDeserializer::BlockInserter::FRONT);
									}
									else
									{
										inserter = PlainDeserializer::BlockInserter(_bundle_reg, PlainDeserializer::BlockInserter::MIDDLE, offset);
									}
								}

								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_CONTINUE << " BUNDLE BLOCK ADD" << std::endl;

								try
								{
									dtn::data::Block &block = PlainDeserializer(_stream).readBlock(inserter, _bundle_reg.get(dtn::data::Bundle::APPDATA_IS_ADMRECORD));
									if (inserter.getAlignment() == PlainDeserializer::BlockInserter::END)
									{
										block.set(dtn::data::Block::LAST_BLOCK, true);
									}
									else
									{
										block.set(dtn::data::Block::LAST_BLOCK, false);
									}
									_stream << ClientHandler::API_STATUS_OK << " BUNDLE BLOCK ADD SUCCESSFUL" << std::endl;
								}
								catch (const PlainDeserializer::PlainDeserializerException &ex){
									_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " BUNDLE BLOCK ADD FAILED" << std::endl;
								}
							}
							else if (cmd[2] == "del")
							{
								if (cmd.size() < 4) throw ibrcommon::Exception("not enough parameters");

								int offset;
								istringstream ss(cmd[3]);

								ss >> offset;
								if (ss.fail()) throw ibrcommon::Exception("malformed command");

								_bundle_reg.remove(_bundle_reg.getBlock(offset));

								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_OK << " BUNDLE BLOCK DEL SUCCESSFUL" << std::endl;
							}
							else
							{
								ibrcommon::MutexLock l(_write_lock);
								_stream << ClientHandler::API_STATUS_BAD_REQUEST << " UNKNOWN COMMAND" << std::endl;
							}
						}
						else
						{
							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_BAD_REQUEST << " UNKNOWN COMMAND" << std::endl;
						}
					}
					else if (cmd[0] == "payload")
					{
						int block_offset;
						int cmd_index = 1;
						dtn::data::Block& b = _bundle_reg.getBlock<dtn::data::PayloadBlock>();

						if (cmd.size() < 2) throw ibrcommon::Exception("not enough parameters");

						istringstream ss(cmd[1]);
						ss >> block_offset;
						if(!ss.fail())
						{
							cmd_index++;
							if (cmd.size() < 3) throw ibrcommon::Exception("not enough parameters");

							if(block_offset < 0 || block_offset >= _bundle_reg.getBlocks().size()){
								throw ibrcommon::Exception("invalid offset");
							}

							b = _bundle_reg.getBlock(block_offset);
						}

						int cmd_remaining = cmd.size() - (cmd_index + 1);
						if (cmd[cmd_index] == "get")
						{
							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_OK << " PAYLOAD GET" << std::endl;

							int payload_offset = 0;
							int length = 0;

							if(cmd_remaining > 0)
							{

								/* read the payload offset */
								ss.clear(); ss.str(cmd[cmd_index+1]); ss >> payload_offset;
								if(payload_offset < 0)
								{
									payload_offset = 0;
								}

								if(cmd_remaining > 1)
								{
									ss.clear(); ss.str(cmd[cmd_index+2]); ss >> length;
								}
							}

							try{
								size_t slength = 0;
								ibrcommon::BLOB::Reference blob = ibrcommon::BLOB::create();

								/* acquire the iostream to lock the blob */
								ibrcommon::BLOB::iostream blob_stream = blob.iostream();
								/* serialize the payload of the bundle into the blob */
								b.serialize(*blob_stream, slength);

								if(length <= 0 || (length + payload_offset) > slength)
								{
									length = slength - payload_offset;
								}

								blob_stream->ignore(payload_offset);

								PlainSerializer(_stream, false).serialize(blob_stream, length);

								_stream << std::endl;

							} catch (const std::exception&) {
								_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " PAYLOAD GET FAILED" << std::endl;
							}
						}
						else if (cmd[cmd_index] == "put")
						{
							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_CONTINUE << " PAYLOAD PUT" << std::endl;

							int payload_offset = 0;
							if(cmd_remaining > 0)
							{

								/* read the payload offset */
								ss.clear(); ss.str(cmd[cmd_index+1]); ss >> payload_offset;
								if(payload_offset < 0)
								{
									payload_offset = 0;
								}
							}

							try
							{
								size_t slength = 0;
								ibrcommon::BLOB::Reference blob = ibrcommon::BLOB::create();

								/* acquire the iostream to lock the blob */
								ibrcommon::BLOB::iostream blob_stream = blob.iostream();
								/* serialize the payload of the bundle into the blob */
								b.serialize(*blob_stream, slength);

								/* move the streams put pointer to the given offset */
								if(payload_offset < slength){
									blob_stream->seekp(payload_offset, ios_base::beg);
								}

								/* write the new data into the blob */
								PlainDeserializer(_stream) >> blob_stream;

								/* write the result into the block */
								b.deserialize(*blob_stream, blob_stream.size());

								_stream << ClientHandler::API_STATUS_OK << " PAYLOAD PUT SUCCESSFUL" << std::endl;

							} catch (const std::exception&) {
								_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " PAYLOAD PUT FAILED" << std::endl;
							}
						}
						else if (cmd[cmd_index] == "append"){
							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_CONTINUE << " PAYLOAD APPEND" << std::endl;

							try {
								size_t slength = 0;
								ibrcommon::BLOB::Reference blob = ibrcommon::BLOB::create();

								/* acquire the iostream to lock the blob */
								ibrcommon::BLOB::iostream blob_stream = blob.iostream();
								/* serialize old and new data into the blob */
								b.serialize(*blob_stream, slength);
								PlainDeserializer(_stream) >> blob_stream;

								/* write the result into the payload of the block */
								b.deserialize(*blob_stream, blob_stream.size());

								_stream << ClientHandler::API_STATUS_OK << " PAYLOAD APPEND SUCCESSFUL" << std::endl;

							} catch (const std::exception&) {
								_stream << ClientHandler::API_STATUS_NOT_ACCEPTABLE << " PAYLOAD APPEND FAILED" << std::endl;
							}

						}
						else if (cmd[cmd_index] == "clear"){

							std::stringstream stream;
							b.deserialize(stream, 0);

							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_OK << " PAYLOAD CLEAR SUCCESSFUL" << std::endl;
						}
						else if (cmd[cmd_index] == "length"){
							ibrcommon::MutexLock l(_write_lock);
							_stream << ClientHandler::API_STATUS_OK << " PAYLOAD LENGTH" << std::endl;
							_stream << "Length: " << b.getLength() << std::endl;
						}
					}
					else if (cmd[0] == "nodename")
					{
						ibrcommon::MutexLock l(_write_lock);
						_stream << ClientHandler::API_STATUS_OK << " NODENAME " << dtn::core::BundleCore::local.getString() << std::endl;
					}
					else
					{
						ibrcommon::MutexLock l(_write_lock);
						_stream << ClientHandler::API_STATUS_BAD_REQUEST << " UNKNOWN COMMAND" << std::endl;
					}
				} catch (const std::exception&) {
					ibrcommon::MutexLock l(_write_lock);
					_stream << ClientHandler::API_STATUS_BAD_REQUEST << " ERROR" << std::endl;
				}
			}
		}

		ExtendedApiHandler::Sender::Sender(ExtendedApiHandler &conn)
		 : _handler(conn)
		{
		}

		ExtendedApiHandler::Sender::~Sender()
		{
			ibrcommon::JoinableThread::join();
		}

		void ExtendedApiHandler::Sender::__cancellation() throw ()
		{
			// abort all blocking calls on the registration object
			_handler._client.getRegistration().abort();
		}

		void ExtendedApiHandler::Sender::finally() throw ()
		{
		}

		void ExtendedApiHandler::Sender::run() throw ()
		{
			Registration &reg = _handler._client.getRegistration();
			try{
				while(_handler.good()){
					try{
						dtn::data::MetaBundle id = reg.receiveMetaBundle();
						_handler._bundle_queue.push(id);
						ibrcommon::MutexLock l(_handler._write_lock);
						_handler._stream << API_STATUS_NOTIFY_BUNDLE << " NOTIFY BUNDLE ";
						sayBundleID(_handler._stream, id);
						_handler._stream << std::endl;

					} catch (const dtn::storage::BundleStorage::NoBundleFoundException&) {
						reg.wait_for_bundle();
					}

					yield();
				}
			} catch (const ibrcommon::QueueUnblockedException &ex) {
				IBRCOMMON_LOGGER_DEBUG(40) << "ExtendedApiHandler::Sender::run(): aborted" << IBRCOMMON_LOGGER_ENDL;
				return;
			} catch (const ibrcommon::IOException &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "API: IOException says " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			} catch (const dtn::InvalidDataException &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "API: InvalidDataException says " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			} catch (const std::exception &ex) {
				IBRCOMMON_LOGGER_DEBUG(10) << "unexpected API error! " << ex.what() << IBRCOMMON_LOGGER_ENDL;
			}

			try {
				//FIXME
//				_handler.stop();
			} catch (const ibrcommon::ThreadException &ex) {
				IBRCOMMON_LOGGER_DEBUG(50) << "ClientHandler::Sender::run(): ThreadException (" << ex.what() << ") on termination" << IBRCOMMON_LOGGER_ENDL;
			}
		}

		void ExtendedApiHandler::sayBundleID(ostream &stream, const dtn::data::BundleID &id)
		{
			stream << id.timestamp << " " << id.sequencenumber << " ";

			if (id.fragment)
			{
				stream << id.offset << " ";
			}

			stream << id.source.getString();
		}

		dtn::data::BundleID ExtendedApiHandler::readBundleID(const std::vector<std::string> &data, const size_t start)
		{
			// load bundle id
			std::stringstream ss;
			size_t timestamp = 0;
			size_t sequencenumber = 0;
			bool fragment = false;
			size_t offset = 0;

			if ((data.size() - start) < 3)
			{
				throw ibrcommon::Exception("not enough parameters");
			}

			// read timestamp
			ss.clear(); ss.str(data[start]); ss >> timestamp;
			if(ss.fail())
			{
			throw ibrcommon::Exception("malformed parameters");
			}

			// read sequence number
			ss.clear(); ss.str(data[start+1]); ss >> sequencenumber;
			if(ss.fail())
			{
			throw ibrcommon::Exception("malformed parameters");
			}

			// read fragment offset
			if ((data.size() - start) > 3)
			{
				fragment = true;

				// read sequence number
				ss.clear(); ss.str(data[start+2]); ss >> offset;
				if(ss.fail())
				{
					throw ibrcommon::Exception("malformed parameters");
				}
			}

			// read EID
			ss.clear(); dtn::data::EID eid(data[data.size() - 1]);

			// construct bundle id
			return dtn::data::BundleID(eid, timestamp, sequencenumber, fragment, offset);
		}
	}
}
