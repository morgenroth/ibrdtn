/*
 * ExtendedApiHandler.h
 *
 *  Created on: 10.10.2011
 *      Author: morgenro,roettger
 */

#ifndef EXTENDEDAPIHANDLER_H_
#define EXTENDEDAPIHANDLER_H_

#include "api/Registration.h"
#include "core/Node.h"
#include "api/ClientHandler.h"

#include <ibrdtn/data/Bundle.h>
#include <ibrcommon/thread/Thread.h>
#include <ibrcommon/thread/Queue.h>
#include <ibrcommon/net/tcpstream.h>


namespace dtn
{
	namespace api
	{
		class ExtendedApiHandler : public ProtocolHandler
		{
		public:
			enum STATUS_CODES
			{
				API_STATUS_NOTIFY_COMMON = 600,
				API_STATUS_NOTIFY_NEIGHBOR = 601,
				API_STATUS_NOTIFY_BUNDLE = 602
			};

			ExtendedApiHandler(ClientHandler &client, ibrcommon::tcpstream &stream);
			virtual ~ExtendedApiHandler();

			virtual void run();
			virtual void finally();
			virtual void __cancellation();

			bool good() const;

//			void eventNodeAvailable(const dtn::core::Node &node);
//			void eventNodeUnavailable(const dtn::core::Node &node);

		private:
			class Sender : public ibrcommon::JoinableThread
			{
			public:
				Sender(ExtendedApiHandler &conn);
				virtual ~Sender();

			protected:
				void run();
				void finally();
				void __cancellation();

			private:
				ExtendedApiHandler &_handler;
			} *_sender;

			static void sayBundleID(ostream &stream, const dtn::data::BundleID &id);
			static dtn::data::BundleID readBundleID(const std::vector<std::string>&, const size_t start);

			void processIncomingBundle(dtn::data::Bundle &b);

			ibrcommon::Mutex _write_lock;

			dtn::data::Bundle _bundle_reg;

			dtn::data::EID _endpoint;
			ibrcommon::Queue<dtn::data::BundleID> _bundle_queue;
		};
	}
}

#endif /* EXTENDEDAPICONNECTION_H_ */
