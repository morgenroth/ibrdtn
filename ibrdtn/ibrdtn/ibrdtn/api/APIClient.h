/*
 * APIClient.h
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

#ifndef APICLIENT_H_
#define APICLIENT_H_

#include "ibrdtn/api/Bundle.h"
#include "ibrdtn/data/BundleID.h"
#include <ibrcommon/net/socketstream.h>
#include <ibrcommon/thread/Queue.h>
#include <ibrcommon/Exceptions.h>

namespace dtn
{
	namespace api
	{
		/**
		 * This is an abstract class is the base for any API connection to a
		 * IBR-DTN daemon. It uses an existing I/O stream to communicate bidirectional
		 * with the daemon.
		 *
		 * For asynchronous reception of bundle this class contains a thread which deals the
		 * receiving part of the communication and calls the received() methods which should be
		 * overwritten.
		 */
		class APIClient
		{
		public:
			enum STATUS_CODES
			{
				API_STATUS_CONTINUE = 100,
				API_STATUS_OK = 200,
				API_STATUS_CREATED = 201,
				API_STATUS_ACCEPTED = 202,
				API_STATUS_FOUND = 302,
				API_STATUS_BAD_REQUEST = 400,
				API_STATUS_UNAUTHORIZED = 401,
				API_STATUS_FORBIDDEN = 403,
				API_STATUS_NOT_FOUND = 404,
				API_STATUS_NOT_ALLOWED = 405,
				API_STATUS_NOT_ACCEPTABLE = 406,
				API_STATUS_CONFLICT = 409,
				API_STATUS_INTERNAL_ERROR = 500,
				API_STATUS_NOT_IMPLEMENTED = 501,
				API_STATUS_SERVICE_UNAVAILABLE = 503,
				API_STATUS_VERSION_NOT_SUPPORTED = 505,
				API_STATUS_NOTIFY_COMMON = 600,
				API_STATUS_NOTIFY_NEIGHBOR = 601,
				API_STATUS_NOTIFY_BUNDLE = 602,
			};

			class Message
			{
			public:
				Message(STATUS_CODES code, const std::string &msg);
				virtual ~Message();

				const STATUS_CODES code;
				const std::string msg;
			};

			APIClient(ibrcommon::socketstream &stream);

			/**
			 * Virtual destructor for this class.
			 */
			virtual ~APIClient();

			void connect();

			/**
			 * set an application endpoint identifier
			 * @param endpoint
			 */
			void setEndpoint(const std::string &endpoint);

			/**
			 * add subscription of a group EID
			 * @param eid
			 */
			void subscribe(const dtn::data::EID &eid);

			/**
			 * remove subscription of a group EID
			 * @param eid
			 */
			void unsubscribe(const dtn::data::EID &eid);

			/**
			 * @return a list of subscriptions
			 */
			std::list<dtn::data::EID> getSubscriptions();

			/**
			 * get a specific bundle
			 * @param bundle
			 */
			virtual dtn::api::Bundle get(dtn::data::BundleID &id);

			/**
			 * Get the next bundle in the queue
			 * @return
			 */
			virtual dtn::api::Bundle get();

			/**
			 * Send a bundle to the daemon
			 * @param bundle
			 */
			virtual void send(const dtn::api::Bundle &bundle);

			/**
			 * wait for a notify
			 */
			Message wait();

			/**
			 * abort all wait() calls
			 */
			void unblock_wait();

			/**
			 * notify the daemon about a delivered bundle
			 * @param id
			 */
			void notify_delivered(const dtn::api::Bundle &b);
			void notify_delivered(const dtn::data::BundleID &id);

		protected:
			/**
			 * upload a bundle into the register
			 * @param bundle
			 */
			virtual void register_put(const dtn::data::Bundle &bundle);

			virtual void register_clear();

			virtual void register_free();

			virtual void register_store();

			virtual void register_send();

			virtual dtn::data::Bundle register_get();

			// receive the response (queue notifies into the queue)
			virtual int __get_return();

			// receive the next notify
			Message __get_message();

			static dtn::data::BundleID readBundleID(const std::string &data);

			// tcp stream reference to send/receive data to the daemon
			ibrcommon::socketstream &_stream;

			// notify queue
			ibrcommon::Conditional _queue_cond;
			bool _get_busy;
			std::queue<Message> _notify_queue;
			std::queue<Message> _status_queue;
		};
	}
}

#endif /* APICLIENT_H_ */
