/**
 * @file HTTPConvergenceLayer.h
 * @date: 08.03.2011
 * @author : Robert Heitz
 *
 * HTTPConvergenceLayer header file. In this file the classes
 * DownloadThread and HTTPConvergenceLayer are defined. The class
 * HTTPConvergenceLayer implements the interfaces ConvergenceLayer
 * and IndependentComponent. The class DownloadThread implemets the
 * JoinableThread interface.
 */


#ifndef HTTPCONVERGENCELAYER_H_
#define HTTPCONVERGENCELAYER_H_

#include "Component.h"

#include "core/BundleCore.h"
#include "core/BundleEvent.h"

#include <ibrdtn/data/Serializer.h>

#include "net/ConvergenceLayer.h"
#include "net/TransferCompletedEvent.h"
#include "net/TransferAbortedEvent.h"
#include "net/BundleReceivedEvent.h"

#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/data/iobuffer.h>

#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/net/vaddress.h>
#include <ibrcommon/thread/MutexLock.h>
#include <ibrcommon/thread/Mutex.h>
#include <ibrcommon/thread/Thread.h>
#include <ibrcommon/Exceptions.h>
#include <ibrcommon/Logger.h>

#include <curl/curl.h>
#include <curl/easy.h>


#include <iostream>

namespace dtn
{
	namespace net
	{
		class HTTPConvergenceLayer : public ConvergenceLayer, public dtn::daemon::IndependentComponent
		{
		public:
			HTTPConvergenceLayer(const std::string &server);
			virtual ~HTTPConvergenceLayer();

			dtn::core::Node::Protocol getDiscoveryProtocol() const;

			/**
			 * @see ConvergenceLayer::queue()
			 */
			void queue(const dtn::core::Node &n, const ConvergenceLayer::Job &job);

			/**
			 * @see Component::getName()
			 */
			virtual const std::string getName() const;


		protected:
			virtual void componentUp();
			virtual void componentRun();
			virtual void componentDown();
			void __cancellation();

		private:
			/** Variable contains Tomcat-Server URL, which is specified
			 * in the IBR-DTN configuration file.
			 */
			const std::string _server;

			/** For managing access for iobuffer */
			ibrcommon::Mutex _push_iob_mutex;
			/** IO buffer in witch the received data will be witten.*/
			ibrcommon::iobuffer *_push_iob;
			/** variable to control the independent thread, when true the thread is running */
			bool _running;
		};

		class DownloadThread : public ibrcommon::JoinableThread
		{
		public:

			DownloadThread(ibrcommon::iobuffer &buf);
			virtual ~DownloadThread();

		protected:
			void run();
			void __cancellation();

		private:
			/** istream variable is using for reading from iobuffer */
			std::istream _stream;
		};

	}
}

#endif /* HTTPCONVERGENCELAYER_H_ */
