/*
 * BinaryStreamClient.h
 *
 *  Created on: 19.07.2011
 *      Author: morgenro
 */

#ifndef BINARYSTREAMCLIENT_H_
#define BINARYSTREAMCLIENT_H_

#include "api/ClientHandler.h"
#include <ibrdtn/streams/StreamConnection.h>
#include <ibrdtn/streams/StreamContactHeader.h>

namespace dtn
{
	namespace api
	{
		class BinaryStreamClient : public dtn::streams::StreamConnection::Callback, public ProtocolHandler
		{
			public:
				BinaryStreamClient(ClientHandler &client, ibrcommon::tcpstream &stream);
				virtual ~BinaryStreamClient();

				virtual void eventShutdown(dtn::streams::StreamConnection::ConnectionShutdownCases csc);
				virtual void eventTimeout();
				virtual void eventError();
				virtual void eventConnectionDown();
				virtual void eventConnectionUp(const dtn::streams::StreamContactHeader &header);

				virtual void eventBundleRefused();
				virtual void eventBundleForwarded();
				virtual void eventBundleAck(size_t ack);

				const dtn::data::EID& getPeer() const;

				void queue(const dtn::data::Bundle &bundle);

				void received(const dtn::streams::StreamContactHeader &h);
				void run();
				void finally();
				void __cancellation();
				bool good() const;

			private:
				class Sender : public ibrcommon::JoinableThread, public ibrcommon::Queue<dtn::data::Bundle>
				{
				public:
					Sender(BinaryStreamClient &client);
					virtual ~Sender();

				protected:
					void run();
					void __cancellation();

					void received(const dtn::streams::StreamContactHeader &h);
					bool good() const;

				private:
					BinaryStreamClient &_client;
				};

				friend class Sender;

				BinaryStreamClient::Sender _sender;

				dtn::streams::StreamConnection _connection;
				dtn::streams::StreamContactHeader _contact;

				ibrcommon::Queue<dtn::data::Bundle> _sentqueue;
				size_t _lastack;

				dtn::data::EID _eid;
		};
	}
}

#endif /* BINARYSTREAMCLIENT_H_ */
