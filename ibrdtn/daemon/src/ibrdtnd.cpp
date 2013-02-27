/*
 * ibrdtnd.cpp
 *
 *  Created on: 05.11.2012
 *      Author: morgenro
 */

#include "config.h"
#include "ibrdtnd.h"

#include <ibrcommon/data/BLOB.h>
#include <ibrcommon/data/File.h>
#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/Logger.h>
#include <ibrcommon/link/LinkManager.h>
#include <ibrdtn/utils/Clock.h>
#include <list>

#include "StandByManager.h"

#include "storage/BundleStorage.h"
#include "storage/BundleSeeker.h"
#include "storage/MemoryBundleStorage.h"
#include "storage/SimpleBundleStorage.h"

#include "core/BundleCore.h"
#include "core/FragmentManager.h"
#include "core/Node.h"
#include "core/EventSwitch.h"
#include "core/GlobalEvent.h"
#include "core/NodeEvent.h"

#include "routing/BaseRouter.h"
#include "routing/StaticRoutingExtension.h"
#include "routing/NeighborRoutingExtension.h"
#include "routing/SchedulingBundleIndex.h"
#include "routing/epidemic/EpidemicRoutingExtension.h"
#include "routing/prophet/ProphetRoutingExtension.h"
#include "routing/flooding/FloodRoutingExtension.h"

#include "net/UDPConvergenceLayer.h"
#include "net/TCPConvergenceLayer.h"
#include "net/FileConvergenceLayer.h"

#ifdef HAVE_SYS_INOTIFY_H
#include "net/FileMonitor.h"
#endif

#include "net/DatagramConvergenceLayer.h"
#include "net/UDPDatagramService.h"

#ifdef HAVE_SQLITE
#include "storage/SQLiteBundleStorage.h"
#endif

#ifdef HAVE_LIBCURL
#include "net/HTTPConvergenceLayer.h"
#endif

#ifdef HAVE_LOWPAN_SUPPORT
#include "net/LOWPANConvergenceLayer.h"
#include "net/LOWPANDatagramService.h"
#endif

#include "net/IPNDAgent.h"

#include "api/ApiServer.h"

#include "Configuration.h"
#include "EchoWorker.h"
#include "CapsuleWorker.h"
#include "DTNTPWorker.h"
#include "DevNull.h"
#include "Component.h"

#ifdef WITH_BUNDLE_SECURITY
#include "security/SecurityManager.h"
#include "security/SecurityKeyManager.h"
#endif

#ifdef WITH_TLS
#include "security/SecurityCertificateManager.h"
#include "security/TLSStreamComponent.h"
#endif

#ifdef WITH_DHT_NAMESERVICE
#include "net/DHTNameService.h"
#endif

#include "Debugger.h"
#include "core/EventDebugger.h"

#define UNIT_MB * 1048576

// list of components
std::list< dtn::daemon::Component* > __ibrdtn_daemon_components;

// stand-by manager
dtn::daemon::StandByManager *__ibrdtn_daemon_standby_manager = NULL;

// IP neighbor discovery process
dtn::net::IPNDAgent *__ibrdtn_daemon_ipnd = NULL;

// bundle index used to search for new bundles in the routing modules
dtn::storage::BundleIndex *_bundle_index = NULL;
dtn::storage::BundleSeeker *_bundle_seeker = NULL;

int ibrdtn_daemon_initialize_blob_storage() {
	dtn::daemon::Configuration &config = dtn::daemon::Configuration::getInstance();

	try {
		// the configured BLOB path
		ibrcommon::File blob_path = config.getPath("blob");

		// check if the BLOB path exists
		if (blob_path.exists())
		{
			if (blob_path.isDirectory())
			{
				IBRCOMMON_LOGGER_TAG("Init", info) << "using BLOB path: " << blob_path.getPath() << IBRCOMMON_LOGGER_ENDL;
				ibrcommon::BLOB::changeProvider(new ibrcommon::FileBLOBProvider(blob_path), false);
			}
			else
			{
				IBRCOMMON_LOGGER_TAG("Init", warning) << "BLOB path exists, but is not a directory! Fallback to memory based mode." << IBRCOMMON_LOGGER_ENDL;
			}
		}
		else
		{
			// try to create the BLOB path
			ibrcommon::File::createDirectory(blob_path);

			if (blob_path.exists())
			{
				IBRCOMMON_LOGGER_TAG("Init", info) << "using BLOB path: " << blob_path.getPath() << IBRCOMMON_LOGGER_ENDL;
				ibrcommon::BLOB::changeProvider(new ibrcommon::FileBLOBProvider(blob_path), false);
			}
			else
			{
				IBRCOMMON_LOGGER_TAG("Init", warning) << "Could not create BLOB path! Fallback to memory based mode." << IBRCOMMON_LOGGER_ENDL;
			}
		}
	} catch (const dtn::daemon::Configuration::ParameterNotSetException&) {
	}

	return 0;
}

int ibrdtn_daemon_initialize_bundle_storage()
{
	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();
	std::list< dtn::daemon::Component* > &components = __ibrdtn_daemon_components;

	dtn::storage::BundleStorage *storage = NULL;

#ifdef HAVE_SQLITE
	if (conf.getStorage() == "sqlite")
	{
		try {
			// new methods for blobs
			ibrcommon::File path = conf.getPath("storage");

			// create workdir if needed
			if (!path.exists())
			{
				ibrcommon::File::createDirectory(path);
			}

			IBRCOMMON_LOGGER_TAG("Init", info) << "using sqlite bundle storage in " << path.getPath() << IBRCOMMON_LOGGER_ENDL;

			dtn::storage::SQLiteBundleStorage *sbs = new dtn::storage::SQLiteBundleStorage(path, conf.getLimit("storage") );

			// use sqlite storage as BLOB provider, auto delete off
			ibrcommon::BLOB::changeProvider(sbs, false);

			components.push_back(sbs);
			storage = sbs;
		} catch (const dtn::daemon::Configuration::ParameterNotSetException&) {
			IBRCOMMON_LOGGER_TAG("Init", error) << "storage for bundles" << IBRCOMMON_LOGGER_ENDL;
			return -1;
		}
	}
#endif

	if ((conf.getStorage() == "simple") || (conf.getStorage() == "default"))
	{
		// default behavior if no bundle storage is set
		try {
			// new methods for blobs
			ibrcommon::File path = conf.getPath("storage");

			// create workdir if needed
			if (!path.exists())
			{
				ibrcommon::File::createDirectory(path);
			}

			IBRCOMMON_LOGGER_TAG("Init", info) << "using simple bundle storage in " << path.getPath() << IBRCOMMON_LOGGER_ENDL;

			dtn::storage::SimpleBundleStorage *sbs = new dtn::storage::SimpleBundleStorage(path, conf.getLimit("storage"), conf.getLimit("storage_buffer"));

			// initialize BLOB mechanism
			ibrdtn_daemon_initialize_blob_storage();

			components.push_back(sbs);
			storage = sbs;
		} catch (const dtn::daemon::Configuration::ParameterNotSetException&) {
			IBRCOMMON_LOGGER_TAG("Init", info) << "using bundle storage in memory-only mode" << IBRCOMMON_LOGGER_ENDL;

			dtn::storage::MemoryBundleStorage *sbs = new dtn::storage::MemoryBundleStorage(conf.getLimit("storage"));

			// initialize BLOB mechanism
			ibrdtn_daemon_initialize_blob_storage();

			components.push_back(sbs);
			storage = sbs;
		}
	}

	if (storage == NULL)
	{
		IBRCOMMON_LOGGER_TAG("Init", error) << "bundle storage module \"" << conf.getStorage() << "\" do not exists!" << IBRCOMMON_LOGGER_ENDL;
		return -1;
	}

	// set the storage in the core
	dtn::core::BundleCore::getInstance().setStorage(storage);

	// use scheduling?
	if (dtn::daemon::Configuration::getInstance().getNetwork().doScheduling())
	{
		// create a new bundle index
		_bundle_index = new dtn::routing::SchedulingBundleIndex();

		// attach index to the storage
		storage->attach(_bundle_index);

		// set this bundle index as default bundle seeker which is used to find bundles to transfer
		_bundle_seeker = static_cast<dtn::storage::BundleSeeker*>(_bundle_index);

		IBRCOMMON_LOGGER_TAG("Init", info) << "Enable extended bundle index for scheduling" << IBRCOMMON_LOGGER_ENDL;
	} else {
		// use default bundle seeker which is used to find bundles to transfer
		_bundle_seeker = storage;
	}

	// set the default seeker in the core
	dtn::core::BundleCore::getInstance().setSeeker(_bundle_seeker);

	return 0;
}

int ibrdtn_daemon_initialize_convergencelayer()
{
	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();
	std::list< dtn::daemon::Component* > &components = __ibrdtn_daemon_components;
	dtn::core::BundleCore &core = dtn::core::BundleCore::getInstance();

	// get the configuration of the convergence layers
	const std::list<dtn::daemon::Configuration::NetConfig> &nets = conf.getNetwork().getInterfaces();

	// local cl map
	std::map<dtn::daemon::Configuration::NetConfig::NetType, dtn::net::ConvergenceLayer*> _cl_map;

	// holder for file convergence layer
	FileConvergenceLayer *filecl = NULL;

	// add file monitor
#ifdef HAVE_SYS_INOTIFY_H
	FileMonitor *fm = NULL;
#endif

	// create the convergence layers
 	for (std::list<dtn::daemon::Configuration::NetConfig>::const_iterator iter = nets.begin(); iter != nets.end(); iter++)
	{
		const dtn::daemon::Configuration::NetConfig &net = (*iter);

		try {
			switch (net.type)
			{
				case dtn::daemon::Configuration::NetConfig::NETWORK_FILE:
				{
					try {
						if (filecl == NULL)
						{
							filecl = new FileConvergenceLayer();
							core.getConnectionManager().add(filecl);
							components.push_back(filecl);
						}

#ifdef HAVE_SYS_INOTIFY_H
						if (net.url.size() > 0)
						{
							ibrcommon::File path(net.url);

							if (path.exists())
							{
								if (fm == NULL)
								{
									fm = new FileMonitor();
									components.push_back(fm);
								}
								ibrcommon::File path(net.url);
								fm->watch(path);
							}
						}
#endif
					} catch (const ibrcommon::Exception &ex) {
						IBRCOMMON_LOGGER_TAG("Init", error) << "Failed to add FileConvergenceLayer: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
					break;
				}

				case dtn::daemon::Configuration::NetConfig::NETWORK_UDP:
				{
					try {
						UDPConvergenceLayer *udpcl = new dtn::net::UDPConvergenceLayer( net.iface, net.port, net.mtu );
						core.getConnectionManager().add(udpcl);
						components.push_back(udpcl);
						if (__ibrdtn_daemon_standby_manager != NULL) __ibrdtn_daemon_standby_manager->adopt(udpcl);
						if (__ibrdtn_daemon_ipnd != NULL) 		__ibrdtn_daemon_ipnd->addService(udpcl);

						IBRCOMMON_LOGGER_TAG("Init", info) << "UDP ConvergenceLayer added on " << net.iface.toString() << ":" << net.port << IBRCOMMON_LOGGER_ENDL;
					} catch (const ibrcommon::Exception &ex) {
						IBRCOMMON_LOGGER_TAG("Init", error) << "Failed to add UDP ConvergenceLayer on " << net.iface.toString() << ": " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}

					break;
				}

				case dtn::daemon::Configuration::NetConfig::NETWORK_TCP:
				{
					// look for an earlier instance of
					std::map<dtn::daemon::Configuration::NetConfig::NetType, dtn::net::ConvergenceLayer*>::iterator it = _cl_map.find(net.type);

					TCPConvergenceLayer *tcpcl = NULL;

					if (it == _cl_map.end()) {
						tcpcl = new TCPConvergenceLayer();
					} else {
						tcpcl = dynamic_cast<TCPConvergenceLayer*>(it->second);
					}

					try {
						tcpcl->add(net.iface, net.port);

						if (it == _cl_map.end()) {
							core.getConnectionManager().add(tcpcl);
							components.push_back(tcpcl);
							if (__ibrdtn_daemon_standby_manager != NULL) __ibrdtn_daemon_standby_manager->adopt(tcpcl);
							if (__ibrdtn_daemon_ipnd != NULL) __ibrdtn_daemon_ipnd->addService(tcpcl);
							_cl_map[net.type] = tcpcl;
						}

						IBRCOMMON_LOGGER_TAG("Init", info) << "TCP ConvergenceLayer added on " << net.iface.toString() << ":" << net.port << IBRCOMMON_LOGGER_ENDL;
					} catch (const ibrcommon::Exception &ex) {
						if (it == _cl_map.end()) {
							delete tcpcl;
						}

						IBRCOMMON_LOGGER_TAG("Init", error) << "Failed to add TCP ConvergenceLayer on " << net.iface.toString() << ": " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}

					break;
				}

#ifdef HAVE_LIBCURL
				case dtn::daemon::Configuration::NetConfig::NETWORK_HTTP:
				{
					try {
						HTTPConvergenceLayer *httpcl = new HTTPConvergenceLayer( net.url );
						core.getConnectionManager().add(httpcl);
						if (__ibrdtn_daemon_standby_manager != NULL) __ibrdtn_daemon_standby_manager->adopt(httpcl);
						components.push_back(httpcl);

						IBRCOMMON_LOGGER_TAG("Init", info) << "HTTP ConvergenceLayer added, Server: " << net.url << IBRCOMMON_LOGGER_ENDL;
					} catch (const ibrcommon::Exception &ex) {
						IBRCOMMON_LOGGER_TAG("Init", error) << "Failed to add HTTP ConvergenceLayer, Server: " << net.url << ": " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
					break;
				}
#endif

#ifdef HAVE_LOWPAN_SUPPORT
				case dtn::daemon::Configuration::NetConfig::NETWORK_LOWPAN:
				{
					try {
						LOWPANConvergenceLayer *lowpancl = new LOWPANConvergenceLayer( net.iface, net.port );
						core.getConnectionManager().add(lowpancl);
						components.push_back(lowpancl);
						if (__ibrdtn_daemon_standby_manager != NULL) __ibrdtn_daemon_standby_manager->adopt(lowpancl);
						if (__ibrdtn_daemon_ipnd != NULL) __ibrdtn_daemon_ipnd->addService(lowpancl);

						IBRCOMMON_LOGGER_TAG("Init", info) << "LOWPAN ConvergenceLayer added on " << net.iface.toString() << ":" << net.port << IBRCOMMON_LOGGER_ENDL;
					} catch (const ibrcommon::Exception &ex) {
						IBRCOMMON_LOGGER_TAG("Init", error) << "Failed to add LOWPAN ConvergenceLayer on " << net.iface.toString() << ": " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}

					break;
				}

				case dtn::daemon::Configuration::NetConfig::NETWORK_DGRAM_LOWPAN:
				{
					try {
						LOWPANDatagramService *lowpan_service = new LOWPANDatagramService( net.iface, net.port );
						DatagramConvergenceLayer *dgram_cl = new DatagramConvergenceLayer(lowpan_service);
						core.getConnectionManager().add(dgram_cl);
						if (__ibrdtn_daemon_standby_manager != NULL) __ibrdtn_daemon_standby_manager->adopt(dgram_cl);
						components.push_back(dgram_cl);

						IBRCOMMON_LOGGER_TAG("Init", info) << "Datagram ConvergenceLayer (LowPAN) added on " << net.iface.toString() << ":" << net.port << IBRCOMMON_LOGGER_ENDL;
					} catch (const ibrcommon::Exception &ex) {
						IBRCOMMON_LOGGER_TAG("Init", error) << "Failed to add Datagram ConvergenceLayer (LowPAN) on " << net.iface.toString() << ": " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
					break;
				}
#endif

				case dtn::daemon::Configuration::NetConfig::NETWORK_DGRAM_UDP:
				{
					try {
						UDPDatagramService *dgram_service = new UDPDatagramService( net.iface, net.port, net.mtu );
						DatagramConvergenceLayer *dgram_cl = new DatagramConvergenceLayer(dgram_service);
						core.getConnectionManager().add(dgram_cl);
						if (__ibrdtn_daemon_standby_manager != NULL) __ibrdtn_daemon_standby_manager->adopt(dgram_cl);
						components.push_back(dgram_cl);

						IBRCOMMON_LOGGER_TAG("Init", info) << "Datagram ConvergenceLayer (UDP) added on " << net.iface.toString() << ":" << net.port << IBRCOMMON_LOGGER_ENDL;
					} catch (const ibrcommon::Exception &ex) {
						IBRCOMMON_LOGGER_TAG("Init", error) << "Failed to add Datagram ConvergenceLayer (UDP) on " << net.iface.toString() << ": " << ex.what() << IBRCOMMON_LOGGER_ENDL;
					}
					break;
				}

				default:
					break;
			}
		} catch (const std::exception &ex) {
			IBRCOMMON_LOGGER_TAG("Init", error) << "Error: " << ex.what() << IBRCOMMON_LOGGER_ENDL;
		}
	}

 	return 0;
}

int ibrdtn_daemon_initialize_security() {
#ifdef WITH_BUNDLE_SECURITY
	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();

	const dtn::daemon::Configuration::Security &sec = conf.getSecurity();

	if (sec.enabled())
	{
		// initialize the key manager for the security extensions
		dtn::security::SecurityKeyManager::getInstance().initialize( sec.getPath(), sec.getCertificate(), sec.getKey() );
	}
#endif

	return 0;
}

int ibrdtn_daemon_initialize_global_variables() {
	dtn::daemon::Configuration &config = dtn::daemon::Configuration::getInstance();

	// set the timezone
	dtn::utils::Clock::setTimezone(config.getTimezone());

	// set local eid
	dtn::core::BundleCore::local = config.getNodename();
	IBRCOMMON_LOGGER_TAG("Init", info) << "Local node name: " << config.getNodename() << IBRCOMMON_LOGGER_ENDL;

	// set block size limit
	dtn::core::BundleCore::blocksizelimit = config.getLimit("blocksize");
	if (dtn::core::BundleCore::blocksizelimit > 0)
	{
		IBRCOMMON_LOGGER_TAG("Init", info) << "Block size limited to " << dtn::core::BundleCore::blocksizelimit << " bytes" << IBRCOMMON_LOGGER_ENDL;
	}

	// set the lifetime limit
	dtn::core::BundleCore::max_lifetime = config.getLimit("lifetime");
	if (dtn::core::BundleCore::max_lifetime > 0)
	{
		IBRCOMMON_LOGGER_TAG("Init", info) << "Lifetime limited to " << dtn::core::BundleCore::max_lifetime << " seconds" << IBRCOMMON_LOGGER_ENDL;
	}

	// set the timestamp limit
	dtn::core::BundleCore::max_timestamp_future = config.getLimit("predated_timestamp");
	if (dtn::core::BundleCore::max_timestamp_future > 0)
	{
		IBRCOMMON_LOGGER_TAG("Init", info) << "Pre-dated timestamp limited to " << dtn::core::BundleCore::max_timestamp_future << " seconds in the future" << IBRCOMMON_LOGGER_ENDL;
	}

	// set the maximum count of bundles in transit (bundles to send to the CL queue)
	size_t transit_limit = config.getLimit("bundles_in_transit");
	if (transit_limit > 0)
	{
		dtn::core::BundleCore::max_bundles_in_transit = transit_limit;
		IBRCOMMON_LOGGER_TAG("Init", info) << "Limit the number of bundles in transit to " << dtn::core::BundleCore::max_bundles_in_transit << IBRCOMMON_LOGGER_ENDL;
	}

	return 0;
}

int ibrdtn_daemon_initialize_components() {
	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();
	std::list< dtn::daemon::Component* > &components = __ibrdtn_daemon_components;

	if (conf.getNetwork().doFragmentation())
	{
		// manager class for fragmentations
		components.push_back( new dtn::core::FragmentManager() );
	}

	// create stand-by manager module
	__ibrdtn_daemon_standby_manager = new dtn::daemon::StandByManager();
	components.push_back( __ibrdtn_daemon_standby_manager);

	return 0;
}

int ibrdtn_daemon_initialize_discovery() {
	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();
	std::list< dtn::daemon::Component* > &components = __ibrdtn_daemon_components;

	if (conf.getDiscovery().enabled())
	{
		// get the discovery port
		int disco_port = conf.getDiscovery().port();

		// collect all interfaces of convergence layer instances
		std::set<ibrcommon::vinterface> interfaces;

		const std::list<dtn::daemon::Configuration::NetConfig> &nets = conf.getNetwork().getInterfaces();
		for (std::list<dtn::daemon::Configuration::NetConfig>::const_iterator iter = nets.begin(); iter != nets.end(); iter++)
		{
			const dtn::daemon::Configuration::NetConfig &net = (*iter);
			if (!net.iface.empty())
				interfaces.insert(net.iface);
		}

		__ibrdtn_daemon_ipnd = new dtn::net::IPNDAgent( disco_port );

		try {
			const std::set<ibrcommon::vaddress> addr = conf.getDiscovery().address();
			for (std::set<ibrcommon::vaddress>::const_iterator iter = addr.begin(); iter != addr.end(); iter++) {
				__ibrdtn_daemon_ipnd->add(*iter);
			}
		} catch (const dtn::daemon::Configuration::ParameterNotFoundException&) {
			// by default set multicast equivalent of broadcast
			__ibrdtn_daemon_ipnd->add(ibrcommon::vaddress("ff02::142", disco_port, AF_INET6));
			__ibrdtn_daemon_ipnd->add(ibrcommon::vaddress("224.0.0.142", disco_port, AF_INET));
		}

		for (std::set<ibrcommon::vinterface>::const_iterator iter = interfaces.begin(); iter != interfaces.end(); iter++)
		{
			const ibrcommon::vinterface &i = (*iter);

			// add interfaces to discovery
			__ibrdtn_daemon_ipnd->bind(i);
		}

		components.push_back(__ibrdtn_daemon_ipnd);
		if (__ibrdtn_daemon_standby_manager != NULL) __ibrdtn_daemon_standby_manager->adopt(__ibrdtn_daemon_ipnd);
	}
	else
	{
		IBRCOMMON_LOGGER_TAG("Init", info) << "Discovery disabled" << IBRCOMMON_LOGGER_ENDL;
	}

	return 0;
}

int ibrdtn_daemon_initialize_routing() {
	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();
	std::list< dtn::daemon::Component* > &components = __ibrdtn_daemon_components;
	dtn::core::BundleCore &core = dtn::core::BundleCore::getInstance();

	// create the base router
	dtn::routing::BaseRouter *router = new dtn::routing::BaseRouter(core.getStorage());

	// make the router globally available
	core.setRouter(router);

	// add static routing extension
	router->addExtension( new dtn::routing::StaticRoutingExtension(*_bundle_seeker) );

	// add neighbor routing (direct-delivery) extension
	router->addExtension( new dtn::routing::NeighborRoutingExtension(*_bundle_seeker) );

	// add other routing extensions depending on the configuration
	switch (conf.getNetwork().getRoutingExtension())
	{
	case dtn::daemon::Configuration::FLOOD_ROUTING:
	{
		IBRCOMMON_LOGGER_TAG("Init", info) << "Using flooding routing extensions" << IBRCOMMON_LOGGER_ENDL;
		router->addExtension( new dtn::routing::FloodRoutingExtension(*_bundle_seeker) );
		break;
	}

	case dtn::daemon::Configuration::EPIDEMIC_ROUTING:
	{
		IBRCOMMON_LOGGER_TAG("Init", info) << "Using epidemic routing extensions" << IBRCOMMON_LOGGER_ENDL;
		router->addExtension( new dtn::routing::EpidemicRoutingExtension(*_bundle_seeker) );
		break;
	}

	case dtn::daemon::Configuration::PROPHET_ROUTING:
	{
		dtn::daemon::Configuration::Network::ProphetConfig prophet_config = conf.getNetwork().getProphetConfig();
		std::string strategy_name = prophet_config.forwarding_strategy;
		dtn::routing::ForwardingStrategy *forwarding_strategy;
		if(strategy_name == "GRTR"){
			forwarding_strategy = new dtn::routing::ProphetRoutingExtension::GRTR_Strategy();
		}
		else if(strategy_name == "GTMX"){
			forwarding_strategy = new dtn::routing::ProphetRoutingExtension::GTMX_Strategy(prophet_config.gtmx_nf_max);
		}
		else{
			IBRCOMMON_LOGGER_TAG("Init", error) << "Prophet forwarding strategy " << strategy_name << " not found. Using GRTR as fallback." << IBRCOMMON_LOGGER_ENDL;
			forwarding_strategy = new dtn::routing::ProphetRoutingExtension::GRTR_Strategy();
		}
		IBRCOMMON_LOGGER_TAG("Init", info) << "Using prophet routing extensions with " << strategy_name << " as forwarding strategy." << IBRCOMMON_LOGGER_ENDL;
		router->addExtension( new dtn::routing::ProphetRoutingExtension(*_bundle_seeker, forwarding_strategy, prophet_config.p_encounter_max,
										prophet_config.p_encounter_first, prophet_config.p_first_threshold,
										prophet_config.beta, prophet_config.gamma, prophet_config.delta,
										prophet_config.time_unit, prophet_config.i_typ,
										prophet_config.next_exchange_timeout));
		break;
	}

	default:
		IBRCOMMON_LOGGER_TAG("Init", info) << "Using default routing extensions" << IBRCOMMON_LOGGER_ENDL;
		break;
	}

	components.push_back(router);

	// enable or disable forwarding of bundles
	if (conf.getNetwork().doForwarding())
	{
		IBRCOMMON_LOGGER_TAG("Init", info) << "Forwarding of bundles enabled." << IBRCOMMON_LOGGER_ENDL;
		BundleCore::forwarding = true;
	}
	else
	{
		IBRCOMMON_LOGGER_TAG("Init", info) << "Forwarding of bundles disabled." << IBRCOMMON_LOGGER_ENDL;
		BundleCore::forwarding = false;
	}

	return 0;
}

int ibrdtn_daemon_initialize_api() {
	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();
	std::list< dtn::daemon::Component* > &components = __ibrdtn_daemon_components;

	if (conf.doAPI())
	{
 		try {
			ibrcommon::File socket = conf.getAPISocket();

			try {
				// use unix domain sockets for API
				components.push_back(new dtn::api::ApiServer(*_bundle_seeker, socket));
				IBRCOMMON_LOGGER_TAG("Init", info) << "API initialized using unix domain socket: " << socket.getPath() << IBRCOMMON_LOGGER_ENDL;
			} catch (const ibrcommon::socket_exception&) {
				IBRCOMMON_LOGGER_TAG("Init", error) << "Unable to bind to unix domain socket " << socket.getPath() << ". API not initialized!" << IBRCOMMON_LOGGER_ENDL;
				exit(-1);
			}
		} catch (const dtn::daemon::Configuration::ParameterNotSetException&) {
			dtn::daemon::Configuration::NetConfig apiconf = conf.getAPIInterface();

			try {
				// instance a API server, first create a socket
				components.push_back(new dtn::api::ApiServer(*_bundle_seeker, apiconf.iface, apiconf.port));
				IBRCOMMON_LOGGER_TAG("Init", info) << "API initialized using tcp socket: " << apiconf.iface.toString() << ":" << apiconf.port << IBRCOMMON_LOGGER_ENDL;
			} catch (const ibrcommon::socket_exception&) {
				IBRCOMMON_LOGGER_TAG("Init", error) << "Unable to bind to " << apiconf.iface.toString() << ":" << apiconf.port << ". API not initialized!" << IBRCOMMON_LOGGER_ENDL;
				exit(-1);
			}
		};
	}
	else
	{
		IBRCOMMON_LOGGER_TAG("Init", info) << "API disabled" << IBRCOMMON_LOGGER_ENDL;
	}

	return 0;
}

int ibrdtn_daemon_initialize() {
	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();
	std::list< dtn::daemon::Component* > &components = __ibrdtn_daemon_components;
	dtn::core::BundleCore &core = dtn::core::BundleCore::getInstance();

	int ret = 0;

	// set global vars
	if ((ret = ibrdtn_daemon_initialize_global_variables()) != 0)
	{
		// error
		return ret;
	}

	if ((ret = ibrdtn_daemon_initialize_security()) != 0)
	{
		// error
		return ret;
	}

	if ((ret = ibrdtn_daemon_initialize_components()) != 0)
	{
		// error
		return ret;
	}

	// create a storage for bundles
	if ((ret = ibrdtn_daemon_initialize_bundle_storage()) != 0)
	{
		// error
		return ret;
	}

	if ((ret = ibrdtn_daemon_initialize_discovery()) != 0)
	{
		// error
		return ret;
	}

	if ((ret = ibrdtn_daemon_initialize_routing()) != 0)
	{
		// error
		return ret;
	}

	// enable link manager
	ibrcommon::LinkManager::initialize();

#ifdef WITH_TLS
	/* enable TLS support */
	if ( conf.getSecurity().doTLS() )
	{
		components.push_back(new dtn::security::TLSStreamComponent());
		components.push_back(new dtn::security::SecurityCertificateManager());
		IBRCOMMON_LOGGER_TAG("Init", info) << "TLS security for TCP convergence layer enabled" << IBRCOMMON_LOGGER_ENDL;
	}
#endif

	if ((ret = ibrdtn_daemon_initialize_convergencelayer()) != 0)
	{
		// error
		return ret;
	}

	if ((ret = ibrdtn_daemon_initialize_api()) != 0)
	{
		// error
		return ret;
	}

#ifdef WITH_DHT_NAMESERVICE
	// create dht naming service if configured
	if (conf.getDHT().enabled()){
		IBRCOMMON_LOGGER_DEBUG_TAG("Init", 50) << "DHT: is enabled!" << IBRCOMMON_LOGGER_ENDL;
		dtn::dht::DHTNameService* dhtns = new dtn::dht::DHTNameService();
		components.push_back(dhtns);
		if (__ibrdtn_daemon_standby_manager != NULL) __ibrdtn_daemon_standby_manager->adopt(dhtns);
		if (__ibrdtn_daemon_ipnd != NULL) __ibrdtn_daemon_ipnd->addService(dhtns);
	}
#endif

	// initialize core component
	core.initialize();

	// initialize the event switch
	dtn::core::EventSwitch::getInstance().initialize();

	/**
	 * initialize all components!
	 */
	for (std::list< dtn::daemon::Component* >::iterator iter = components.begin(); iter != components.end(); iter++ )
	{
		IBRCOMMON_LOGGER_DEBUG_TAG("Init", 20) << "Initialize component " << (*iter)->getName() << IBRCOMMON_LOGGER_ENDL;
		(*iter)->initialize();
	}

	return 0;
}

int ibrdtn_daemon_main_loop()
{
	dtn::daemon::Configuration &conf = dtn::daemon::Configuration::getInstance();
	std::list< dtn::daemon::Component* > &components = __ibrdtn_daemon_components;
	dtn::core::BundleCore &core = dtn::core::BundleCore::getInstance();

	// run core component
	core.startup();

	/**
	 * run all components!
	 */
	for (std::list< dtn::daemon::Component* >::iterator iter = components.begin(); iter != components.end(); iter++ )
	{
		IBRCOMMON_LOGGER_DEBUG_TAG("Init", 20) << "Startup component " << (*iter)->getName() << IBRCOMMON_LOGGER_ENDL;
		(*iter)->startup();
	}

	// create event debugger
	dtn::core::EventDebugger event_debugger;

	// Debugger
	dtn::daemon::Debugger debugger;

	// add echo module
	dtn::daemon::EchoWorker echo;

	// add bundle-in-bundle endpoint
	dtn::daemon::CapsuleWorker capsule;

	// add DT-NTP worker
	dtn::daemon::DTNTPWorker dtntp;
	if (__ibrdtn_daemon_ipnd != NULL) __ibrdtn_daemon_ipnd->addService(&dtntp);

	// add DevNull module
	dtn::daemon::DevNull devnull;

	// announce static nodes, create a list of static nodes
	std::list<Node> static_nodes = conf.getNetwork().getStaticNodes();

	for (list<Node>::iterator iter = static_nodes.begin(); iter != static_nodes.end(); iter++)
	{
		core.getConnectionManager().add(*iter);
	}

	if (conf.getDaemon().getThreads() > 1)
	{
		IBRCOMMON_LOGGER_TAG("Init", info) << "Parallel event processing enabled using " << conf.getDaemon().getThreads() << " processes." << IBRCOMMON_LOGGER_ENDL;
	}

	IBRCOMMON_LOGGER_TAG("Init", info) << "daemon ready" << IBRCOMMON_LOGGER_ENDL;

	// create the event switch object
	dtn::core::EventSwitch &esw = dtn::core::EventSwitch::getInstance();

	// run the event switch loop forever
	if (conf.getDaemon().getThreads() > 1)
	{
		esw.loop( conf.getDaemon().getThreads() );
	}
	else
	{
		esw.loop();
	}

	/**
	 * terminate all components!
	 */
	for (std::list< dtn::daemon::Component* >::iterator iter = components.begin(); iter != components.end(); iter++ )
	{
		IBRCOMMON_LOGGER_DEBUG_TAG("shutdown", 20) << "Terminate component " << (*iter)->getName() << IBRCOMMON_LOGGER_ENDL;
		(*iter)->terminate();
	}

	// terminate event switch component
	esw.terminate();

	// terminate core component
	core.terminate();

	// delete bundle index
	if (_bundle_index != NULL)
	{
		core.getStorage().detach(_bundle_index);
		delete _bundle_index;
		_bundle_index = NULL;
		_bundle_seeker = NULL;
	}

	// delete all components
	for (std::list< dtn::daemon::Component* >::iterator iter = components.begin(); iter != components.end(); iter++ )
	{
		delete (*iter);
	}

	// clear the list of all components
	components.clear();

	IBRCOMMON_LOGGER_TAG("shutdown", info) << "shutdown complete" << IBRCOMMON_LOGGER_ENDL;

	return 0;
}

int ibrdtn_daemon_shutdown() {
	dtn::core::GlobalEvent::raise(dtn::core::GlobalEvent::GLOBAL_SHUTDOWN);
	dtn::core::EventSwitch::getInstance().shutdown();
	return 0;
}

int ibrdtn_daemon_runtime_debug(bool val) {
	if (val) {
		// activate debugging
		ibrcommon::Logger::setVerbosity(99);
		IBRCOMMON_LOGGER_TAG("Init", info) << "debug level set to 99" << IBRCOMMON_LOGGER_ENDL;
	} else {
		// de-activate debugging
		ibrcommon::Logger::setVerbosity(0);
		IBRCOMMON_LOGGER_TAG("Init", info) << "debug level set to 0" << IBRCOMMON_LOGGER_ENDL;
	}
	return 0;
}

int ibrdtn_daemon_reload() {
	// reload logger
	ibrcommon::Logger::reload();

	// send reload signal to all modules
	dtn::core::GlobalEvent::raise(dtn::core::GlobalEvent::GLOBAL_RELOAD);

	return 0;
}
