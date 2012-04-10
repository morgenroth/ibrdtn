/*
 * OrderedStreamHandler.h
 *
 *  Created on: 17.11.2011
 *      Author: morgenro
 */

#ifndef ORDEREDSTREAMHANDLER_H_
#define ORDEREDSTREAMHANDLER_H_

#include "api/Registration.h"
#include "api/ClientHandler.h"
#include "api/BundleStreamBuf.h"

#include <iostream>

namespace dtn
{
	namespace api
	{
		class OrderedStreamHandler : public ProtocolHandler, public BundleStreamBufCallback
		{
		public:
			OrderedStreamHandler(ClientHandler &client, ibrcommon::tcpstream &stream);
			virtual ~OrderedStreamHandler();

			virtual void run();
			virtual void finally();
			virtual void __cancellation();

			virtual void put(dtn::data::Bundle &b);
			virtual dtn::data::MetaBundle get(size_t timeout = 0);
			virtual void delivered(const dtn::data::MetaBundle &m);

		private:
			class Sender : public ibrcommon::JoinableThread
			{
			public:
				Sender(OrderedStreamHandler &conn);
				virtual ~Sender();

			protected:
				void run();
				void finally();
				void __cancellation();

			private:
				OrderedStreamHandler &_handler;
			} _sender;

			BundleStreamBuf _streambuf;
			std::iostream _bundlestream;

			dtn::data::EID _peer;
			dtn::data::EID _endpoint;
			bool _group;
			size_t _lifetime;
		};
	} /* namespace api */
} /* namespace dtn */
#endif /* ORDEREDSTREAMHANDLER_H_ */
