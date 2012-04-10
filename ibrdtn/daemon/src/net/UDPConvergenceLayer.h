#ifndef UDPCONVERGENCELAYER_H_
#define UDPCONVERGENCELAYER_H_

#include "Component.h"
#include "net/ConvergenceLayer.h"
#include <ibrcommon/Exceptions.h>
#include "net/DiscoveryServiceProvider.h"
#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/net/udpsocket.h>

using namespace dtn::data;


namespace dtn
{
	namespace net
	{
		/**
		 * This class implement a ConvergenceLayer for UDP/IP.
		 * Each bundle is sent in exact one UDP datagram.
		 */
		class UDPConvergenceLayer : public ConvergenceLayer, public dtn::daemon::IndependentComponent, public DiscoveryServiceProvider
		{
		public:
			/**
			 * Constructor
			 * @param[in] bind_addr The address to bind.
			 * @param[in] port The udp port to use.
			 * @param[in] broadcast If true, the broadcast feature for this socket is enabled.
			 * @param[in] mtu The maximum bundle size.
			 */
			UDPConvergenceLayer(ibrcommon::vinterface net, int port, unsigned int mtu = 1280);

			/**
			 * Desktruktor
			 */
			virtual ~UDPConvergenceLayer();

			/**
			 * this method updates the given values
			 */
			void update(const ibrcommon::vinterface &iface, std::string &name, std::string &data) throw(dtn::net::DiscoveryServiceProvider::NoServiceHereException);

			dtn::core::Node::Protocol getDiscoveryProtocol() const;

			void queue(const dtn::core::Node &n, const ConvergenceLayer::Job &job);

			UDPConvergenceLayer& operator>>(dtn::data::Bundle&);

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
			ibrcommon::udpsocket *_socket;

			ibrcommon::vinterface _net;
			int _port;
			int m_socket;

			static const int DEFAULT_PORT;

			unsigned int m_maxmsgsize;

			ibrcommon::Mutex m_writelock;
			ibrcommon::Mutex m_readlock;

			bool _running;


		};
	}
}

#endif /*UDPCONVERGENCELAYER_H_*/
