/*
 * Configuration.cpp
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

#include "config.h"
#include "Configuration.h"
#include "net/DiscoveryAnnouncement.h"
#include "net/DiscoveryAnnouncement.h"
#include "core/Node.h"

#include <ibrdtn/utils/Utils.h>
#include <ibrdtn/utils/Clock.h>

#include <ibrcommon/net/vinterface.h>
#include <ibrcommon/Logger.h>

#include <getopt.h>
#include <unistd.h>

#ifdef __DEVELOPMENT_ASSERTIONS__
#include <cassert>
#endif

using namespace dtn::net;
using namespace dtn::core;
using namespace dtn::utils;
using namespace ibrcommon;

namespace dtn
{
	namespace daemon
	{
		Configuration::NetConfig::NetConfig(std::string n, NetType t, const std::string &u, bool d)
		 : name(n), type(t), url(u), mtu(0), port(0), discovery(d)
		{
		}

		Configuration::NetConfig::NetConfig(std::string n, NetType t, const ibrcommon::vinterface &i, int p, bool d)
		 : name(n), type(t), interface(i), port(p), discovery(d)
		{
		}

		Configuration::NetConfig::NetConfig(std::string n, NetType t, const ibrcommon::vaddress &a, int p, bool d)
		 : name(n), type(t), interface(), address(a), port(p), discovery(d)
		{
		}

		Configuration::NetConfig::NetConfig(std::string n, NetType t, int p, bool d)
		 : name(n), type(t), interface(), port(p), discovery(d)
		{
		}

		Configuration::NetConfig::~NetConfig()
		{
		}

		std::string Configuration::version()
		{
			std::stringstream ss;
			ss << PACKAGE_VERSION;
		#ifdef SVN_REVISION
			ss << " (build " << SVN_REVISION << ")";
		#endif

			return ss.str();
		}

		Configuration::Configuration()
		 : _filename("config.ini"), _doapi(true)
		{
		}

		Configuration::~Configuration()
		{}

		Configuration::Discovery::Discovery()
		 : _enabled(true), _timeout(5), _crosslayer(false) {};

		Configuration::Statistic::Statistic() {};

		Configuration::Debug::Debug()
		 : _enabled(false), _quiet(false), _level(0) {};

		Configuration::Logger::Logger()
		 : _quiet(false), _options(0), _timestamps(false) {};

		Configuration::Network::Network()
		 : _routing("default"), _forwarding(true), _tcp_nodelay(true), _tcp_chunksize(4096), _default_net("lo"), _use_default_net(false), _auto_connect(0), _fragmentation(false)
		{};

		Configuration::Security::Security()
		 : _enabled(false), _tlsEnabled(false), _tlsRequired(false)
		{};

		Configuration::Daemon::Daemon()
		 : _daemonize(false), _kill(false), _threads(0)
		{};

		Configuration::TimeSync::TimeSync()
		 : _reference(true), _sync(false), _discovery(false)
		{};

		Configuration::DHT::DHT()
		 : _enabled(true), _port(0), _ipv4(true), _ipv6(true), _blacklist(true),
			_minRating(1), _allowNeighbourToAnnounceMe(true), _allowNeighbourAnnouncement(true),
			_ignoreDHTNeighbourInformations(false)
		{};

		Configuration::Discovery::~Discovery() {};
		Configuration::Statistic::~Statistic() {};
		Configuration::Debug::~Debug() {};
		Configuration::Logger::~Logger() {};
		Configuration::Network::~Network() {};
		Configuration::Daemon::~Daemon() {};
		Configuration::TimeSync::~TimeSync() {};
		Configuration::DHT::~DHT() {};

		const Configuration::Discovery& Configuration::getDiscovery() const
		{
			return _disco;
		}

		const Configuration::Statistic& Configuration::getStatistic() const
		{
			return _stats;
		}

		const Configuration::Debug& Configuration::getDebug() const
		{
			return _debug;
		}

		const Configuration::Logger& Configuration::getLogger() const
		{
			return _logger;
		}

		const Configuration::Network& Configuration::getNetwork() const
		{
			return _network;
		}

		const Configuration::Security& Configuration::getSecurity() const
		{
			return _security;
		}

		const Configuration::Daemon& Configuration::getDaemon() const
		{
			return _daemon;
		}

		const Configuration::TimeSync& Configuration::getTimeSync() const
		{
			return _timesync;
		}

		const Configuration::DHT& Configuration::getDHT() const
		{
			return _dht;
		}

		Configuration& Configuration::getInstance()
		{
			static Configuration conf;
			return conf;
		}

		void Configuration::params(int argc, char *argv[])
		{
			int c;
			int doapi = _doapi;
			int disco = _disco._enabled;
			int badclock = dtn::utils::Clock::badclock;
			int timestamp = _logger._timestamps;

			while (1)
			{
				static struct option long_options[] =
				{
						/* These options set a flag. */
						{"noapi", no_argument, &doapi, 0},
						{"nodiscovery", no_argument, &disco, 0},
						{"badclock", no_argument, &badclock, 1},
						{"timestamp", no_argument, &timestamp, 1},

						/* These options don't set a flag. We distinguish them by their indices. */
						{"help", no_argument, 0, 'h'},
#ifdef HAVE_LIBDAEMON
						{"daemon", no_argument, 0, 'D'},
						{"kill", no_argument, 0, 'k'},
						{"pidfile", required_argument, 0, 'p'},
#endif

						{"quiet", no_argument, 0, 'q'},
						{"version", no_argument, 0, 'v'},
						{"interface", required_argument, 0, 'i'},
						{"configuration", required_argument, 0, 'c'},
						{"debug", required_argument, 0, 'd'},
						{0, 0, 0, 0}
				};

				/* getopt_long stores the option index here. */
				int option_index = 0;

#ifdef HAVE_LIBDAEMON
				c = getopt_long (argc, argv, "qhDkp:vi:c:d:t:",
						long_options, &option_index);
#else
				c = getopt_long (argc, argv, "qhvi:c:d:t:",
						long_options, &option_index);
#endif

				/* Detect the end of the options. */
				if (c == -1)
					break;

				switch (c)
				{
				case 0:
					/* If this option set a flag, do nothing else now. */
					if (long_options[option_index].flag != 0)
						break;
					printf ("option %s", long_options[option_index].name);
					if (optarg)
						printf (" with arg %s", optarg);
					printf ("\n");
					break;

				case 'h':
					std::cout << "IBR-DTN version: " << version() << std::endl;
					std::cout << "Syntax: dtnd [options]"  << std::endl;
					std::cout << " -h|--help       display this text" << std::endl;
					std::cout << " -c <file>       set a configuration file" << std::endl;
#ifdef HAVE_LIBDAEMON
					std::cout << " -D              daemonize the process" << std::endl;
					std::cout << " -k              stop the running daemon" << std::endl;
					std::cout << " -p <file>       store the pid in this pidfile" << std::endl;
#endif
					std::cout << " -i <interface>  interface to bind on (e.g. eth0)" << std::endl;
					std::cout << " -d <level>      enable debugging and set a verbose level" << std::endl;
					std::cout << " -q              enables the quiet mode (no logging to the console)" << std::endl;
					std::cout << " -t <threads>    specify a number of threads for parallel event processing" << std::endl;
					std::cout << " --noapi         disable API module" << std::endl;
					std::cout << " --nodiscovery   disable discovery module" << std::endl;
					std::cout << " --badclock      assume a bad clock on the system (use AgeBlock)" << std::endl;
					std::cout << " --timestamp     enables timestamps for logging instead of datetime values" << std::endl;
					exit(0);
					break;

				case 'v':
					std::cout << "IBR-DTN version: " << version() << std::endl;
					exit(0);
					break;

				case 'q':
					_debug._quiet = true;
					break;

				case 'c':
					_filename = optarg;
					break;

				case 'i':
					_network._default_net = ibrcommon::vinterface(optarg);
					_network._use_default_net = true;
					break;

				case 'd':
					_debug._enabled = true;
					_debug._level = atoi(optarg);
					break;

				case 'D':
					_daemon._daemonize = true;
					_debug._quiet = true;
					break;

				case 'k':
					_daemon._daemonize = true;
					_daemon._kill = true;
					break;

				case 'p':
					_daemon._pidfile = std::string(optarg);
					break;

				case 't':
					_daemon._threads = atoi(optarg);
					break;

				case '?':
					/* getopt_long already printed an error message. */
					break;

				default:
					abort();
					break;
				}
			}

			_doapi = doapi;
			_disco._enabled = disco;
			dtn::utils::Clock::badclock = badclock;
			_logger._timestamps = timestamp;
		}

		void Configuration::load()
		{
			load(_filename);
		}

		void Configuration::load(string filename)
		{
			try {
				// load main configuration
				_conf = ibrcommon::ConfigFile(filename);
				_filename = filename;

				IBRCOMMON_LOGGER(info) << "Configuration: " << filename << IBRCOMMON_LOGGER_ENDL;
			} catch (const ibrcommon::ConfigFile::file_not_found&) {
				IBRCOMMON_LOGGER(info) << "Using default settings. Call with --help for options." << IBRCOMMON_LOGGER_ENDL;
				_conf = ConfigFile();

				// set the default user to nobody
				_conf.add<std::string>("user", "nobody");
			}

			// load all configuration extensions
			_disco.load(_conf);
			_stats.load(_conf);
			_debug.load(_conf);
			_logger.load(_conf);
			_network.load(_conf);
			_security.load(_conf);
			_timesync.load(_conf);
			_dht.load(_conf);
		}

		void Configuration::Discovery::load(const ibrcommon::ConfigFile &conf)
		{
			_timeout = conf.read<unsigned int>("discovery_timeout", 5);
			_crosslayer = conf.read<bool>("discovery_crosslayer", true);
		}

		void Configuration::Statistic::load(const ibrcommon::ConfigFile&)
		{
		}

		void Configuration::Logger::load(const ibrcommon::ConfigFile &conf)
		{
			try {
				_logfile = conf.read<std::string>("logfile");
			} catch (const ibrcommon::ConfigFile::key_not_found&) {
			}
		}

		void Configuration::Debug::load(const ibrcommon::ConfigFile&)
		{
		}

		void Configuration::Daemon::load(const ibrcommon::ConfigFile&)
		{
		}

		void Configuration::TimeSync::load(const ibrcommon::ConfigFile &conf)
		{
			try {
				_reference = (conf.read<std::string>("time_reference") == "yes");
			} catch (const ibrcommon::ConfigFile::key_not_found&) { };

			try {
				_sync = (conf.read<std::string>("time_sync_on_discovery") == "yes");
			} catch (const ibrcommon::ConfigFile::key_not_found&) { };

			try {
				_discovery = (conf.read<std::string>("time_discovery_announcements") == "yes");
			} catch (const ibrcommon::ConfigFile::key_not_found&) { };

			_sigma = conf.read<float>("time_sigma", 1.001);
			_psi = conf.read<float>("time_psi", 0.9);
			_sync_level = conf.read<float>("time_sync_level", 0.15);

			// enable the clock modify feature
			dtn::utils::Clock::modify_clock = (conf.read<std::string>("time_set_clock", "no") == "yes");
		}
		
		void Configuration::DHT::load(const ibrcommon::ConfigFile &conf)
		{
			_enabled = (conf.read<std::string> ("dht_enabled", "no") == "yes");
			_port = conf.read<int> ("dht_port", 9999);
			_id = conf.read<string> ("dht_id", "");
			_blacklist = (conf.read<std::string> ("dht_blacklist", "yes") == "yes");
			_selfannounce = (conf.read<std::string> ("dht_self_announce", "yes") == "yes");
			_dnsbootstrapping = (conf.read<std::string> ("dht_bootstrapping", "yes") == "yes");
			string list = conf.read<string> ("dht_bootstrapping_domains", "");
			_bootstrappingdomains = dtn::utils::Utils::tokenize(" ", list);
			list = conf.read<string> ("dht_bootstrapping_ips", "");
			_bootstrappingips = dtn::utils::Utils::tokenize(";", list);
			_ipv4bind = conf.read<string> ("dht_bind_ipv4", "");
			_ipv6bind = conf.read<string> ("dht_bind_ipv6", "");
			_nodesFilePath = conf.read<string> ("dht_nodes_file", "");
			_ipv4 = (conf.read<std::string> ("dht_enable_ipv4", "yes") == "yes");
			_ipv6 = (conf.read<std::string> ("dht_enable_ipv6", "yes") == "yes");
			_minRating = conf.read<int> ("dht_min_rating", 1);
			_allowNeighbourToAnnounceMe = (conf.read<std::string> ("dht_allow_neighbours_to_announce_me", "yes") == "yes");
			_allowNeighbourAnnouncement = (conf.read<std::string> ("dht_allow_neighbour_announcement", "yes") == "yes");
			_ignoreDHTNeighbourInformations = (conf.read<std::string> ("dht_ignore_neighbour_informations", "no") == "yes");
			
			if (_minRating < 0)	_minRating = 0;
		}

		bool Configuration::Debug::quiet() const
		{
			return _quiet;
		}

		bool Configuration::Debug::enabled() const
		{
			return _enabled;
		}

		int Configuration::Debug::level() const
		{
			return _level;
		}

		std::string Configuration::getNodename()
		{
			try {
				return _conf.read<string>("local_uri");
			} catch (const ibrcommon::ConfigFile::key_not_found&) {
				char *hostname_array = new char[64];
				if ( gethostname(hostname_array, 64) != 0 )
				{
					// error
					delete[] hostname_array;
					return "local";
				}

				string hostname(hostname_array);
				delete[] hostname_array;

				stringstream ss;
				ss << "dtn://" << hostname;
				ss >> hostname;

				return hostname;
			}
		}

		const std::list<Configuration::NetConfig>& Configuration::Network::getInterfaces() const
		{
			return _interfaces;
		}

		const std::set<ibrcommon::vaddress> Configuration::Discovery::address() const throw (ParameterNotFoundException)
		{
			std::set<ibrcommon::vaddress> ret;

			try {
				std::string address_str = Configuration::getInstance()._conf.read<string>("discovery_address");
				std::vector<std::string> addresses = dtn::utils::Utils::tokenize(" ", address_str);

				for (std::vector<std::string>::iterator iter = addresses.begin(); iter != addresses.end(); iter++) {
					ret.insert( ibrcommon::vaddress(*iter, port()) );
				}
			} catch (const ConfigFile::key_not_found&) {
				throw ParameterNotFoundException();
			}

			return ret;
		}

		int Configuration::Discovery::port() const
		{
			return Configuration::getInstance()._conf.read<int>("discovery_port", 4551);
		}

		unsigned int Configuration::Discovery::timeout() const
		{
			return _timeout;
		}

		bool Configuration::Discovery::enableCrosslayer() const
		{
			return _crosslayer;
		}

		Configuration::NetConfig Configuration::getAPIInterface()
		{
			size_t port = 4550;
			std::string interface_name = "lo";

			try {
				port = _conf.read<size_t>("api_port");
			} catch (const ConfigFile::key_not_found&) { };

			try {
				interface_name = _conf.read<std::string>("api_interface");
			} catch (const ConfigFile::key_not_found&) { };

			if (interface_name == "any")
			{
				return Configuration::NetConfig("api", Configuration::NetConfig::NETWORK_TCP, ibrcommon::vinterface(), port);
			}

			return Configuration::NetConfig("api", Configuration::NetConfig::NETWORK_TCP, ibrcommon::vinterface(interface_name), port);
		}

		ibrcommon::File Configuration::getAPISocket()
		{
			try {
				return ibrcommon::File(_conf.read<std::string>("api_socket"));
			} catch (const ConfigFile::key_not_found&) {
				throw ParameterNotSetException();
			}

			throw ParameterNotSetException();
		}

		std::string Configuration::getStorage() const
		{
			return _conf.read<std::string>("storage", "default");
		}

		void Configuration::Network::load(const ibrcommon::ConfigFile &conf)
		{
			/**
			 * Load static routes
			 */
			_static_routes.clear();

			string key = "route1";
			unsigned int keynumber = 1;

			while (conf.keyExists( key ))
			{
				vector<string> route = dtn::utils::Utils::tokenize(" ", conf.read<string>(key, "dtn:none dtn:none"));
				_static_routes.insert( pair<std::string, std::string>( route.front(), route.back() ) );

				keynumber++;
				stringstream ss; ss << "route" << keynumber; ss >> key;
			}

			/**
			 * load static nodes
			 */
			// read the node count
			int count = 1;

			// initial prefix
			std::string prefix = "static1_";

			while ( conf.keyExists(prefix + "uri") )
			{
				const dtn::data::EID node_eid( conf.read<std::string>(prefix + "uri", "dtn:none") );

				// create a address URI
				std::stringstream ss;
				ss << "ip=" << conf.read<std::string>(prefix + "address", "127.0.0.1") << ";port=" << conf.read<unsigned int>(prefix + "port", 4556) << ";";

				dtn::core::Node::Protocol p = Node::CONN_UNDEFINED;

				const std::string protocol = conf.read<std::string>(prefix + "proto", "tcp");
				if (protocol == "tcp") p = Node::CONN_TCPIP;
				if (protocol == "udp") p = Node::CONN_UDPIP;
				if (protocol == "lowpan") p = Node::CONN_LOWPAN;
				if (protocol == "zigbee") p = Node::CONN_LOWPAN; //Legacy: Stay compatible with older config files
				if (protocol == "bluetooth") p = Node::CONN_BLUETOOTH;
				if (protocol == "http") p = Node::CONN_HTTP;
				if (protocol == "file") p = Node::CONN_FILE;
				if (protocol == "dgram:udp") p = Node::CONN_DGRAM_UDP;
				if (protocol == "dgram:ethernet") p = Node::CONN_DGRAM_ETHERNET;
				if (protocol == "dgram:lowpan") p = Node::CONN_DGRAM_LOWPAN;

				bool node_exists = false;

				// get node
				for (std::list<Node>::iterator iter = _nodes.begin(); iter != _nodes.end(); iter++)
				{
					dtn::core::Node &n = (*iter);

					if (n.getEID() == node_eid)
					{
						n.add( dtn::core::Node::URI( dtn::core::Node::NODE_STATIC, p, ss.str(), 0, 10 ) );
						n.setConnectImmediately( conf.read<std::string>(prefix + "immediately", "no") == "yes" );
						node_exists = true;
						break;
					}
				}

				if (!node_exists)
				{
					Node n(node_eid);
					n.add( dtn::core::Node::URI( dtn::core::Node::NODE_STATIC, p, ss.str(), 0, 10 ) );
					n.setConnectImmediately( conf.read<std::string>(prefix + "immediately", "no") == "yes" );
					_nodes.push_back(n);
				}

				count++;

				std::stringstream prefix_stream;
				prefix_stream << "static" << count << "_";
				prefix = prefix_stream.str();
			}

			/**
			 * get routing extension
			 */
			_routing = conf.read<string>("routing", "default");

			if(_routing == "prophet"){
				/* read prophet parameters */
				_prophet_config.p_encounter_max = conf.read<float>("prophet_p_encounter_max", 0.7);
				if(_prophet_config.p_encounter_max > 1 || _prophet_config.p_encounter_max <= 0)
					_prophet_config.p_encounter_max = 0.7;
				_prophet_config.p_encounter_first = conf.read<float>("prophet_p_encounter_first", 0.5);
				if(_prophet_config.p_encounter_first > 1 || _prophet_config.p_encounter_first <= 0)
					_prophet_config.p_encounter_first = 0.5;
				_prophet_config.p_first_threshold = conf.read<float>("prophet_p_first_threshold", 0.1);
				if(_prophet_config.p_first_threshold < 0 || _prophet_config.p_first_threshold >= _prophet_config.p_encounter_first)
					_prophet_config.p_first_threshold = 0; //disable first threshold on misconfiguration
				_prophet_config.beta = conf.read<float>("prophet_beta", 0.9);
				if(_prophet_config.beta < 0 || _prophet_config.beta > 1)
					_prophet_config.beta = 0.9;
				_prophet_config.gamma = conf.read<float>("prophet_gamma", 0.999);
				if(_prophet_config.gamma <= 0 || _prophet_config.gamma > 1)
					_prophet_config.gamma = 0.999;
				_prophet_config.delta = conf.read<float>("prophet_delta", 0.01);
				if(_prophet_config.delta < 0 || _prophet_config.delta > 1)
					_prophet_config.delta = 0.01;
				_prophet_config.time_unit = conf.read<ibrcommon::Timer::time_t>("prophet_time_unit", 1);
				if(_prophet_config.time_unit < 1)
					_prophet_config.time_unit = 1;
				_prophet_config.i_typ = conf.read<ibrcommon::Timer::time_t>("prophet_i_typ", 300);
				if(_prophet_config.i_typ < 1)
					_prophet_config.i_typ = 1;
				_prophet_config.next_exchange_timeout = conf.read<ibrcommon::Timer::time_t>("prophet_next_exchange_timeout", 60);
				_prophet_config.forwarding_strategy = conf.read<std::string>("prophet_forwarding_strategy", "GRTR");
				_prophet_config.gtmx_nf_max = conf.read<unsigned int>("prophet_gtmx_nf_max", 30);
			}

			/**
			 * get the routing extension
			 */
			_forwarding = (conf.read<std::string>("routing_forwarding", "yes") == "yes");

			/**
			 * get network interfaces
			 */
			_interfaces.clear();

			if (_use_default_net)
			{
				_interfaces.push_back( Configuration::NetConfig("default", Configuration::NetConfig::NETWORK_TCP, _default_net, 4556) );
			}
			else try
			{
				vector<string> nets = dtn::utils::Utils::tokenize(" ", conf.read<string>("net_interfaces") );
				for (vector<string>::const_iterator iter = nets.begin(); iter != nets.end(); iter++)
				{
					std::string netname = (*iter);

					std::string key_type = "net_" + netname + "_type";
					std::string key_port = "net_" + netname + "_port";
					std::string key_interface = "net_" + netname + "_interface";
					std::string key_address = "net_" + netname + "_address";
					std::string key_discovery = "net_" + netname + "_discovery";
					std::string key_path = "net_" + netname + "_path";
					std::string key_mtu = "net_" + netname + "_mtu";

					std::string type_name = conf.read<string>(key_type, "tcp");
					Configuration::NetConfig::NetType type = Configuration::NetConfig::NETWORK_UNKNOWN;

					if (type_name == "tcp") type = Configuration::NetConfig::NETWORK_TCP;
					if (type_name == "udp") type = Configuration::NetConfig::NETWORK_UDP;
					if (type_name == "http") type = Configuration::NetConfig::NETWORK_HTTP;
					if (type_name == "lowpan") type = Configuration::NetConfig::NETWORK_LOWPAN;
					if (type_name == "file") type = Configuration::NetConfig::NETWORK_FILE;
					if (type_name == "dgram:udp") type = Configuration::NetConfig::NETWORK_DGRAM_UDP;
					if (type_name == "dgram:lowpan") type = Configuration::NetConfig::NETWORK_DGRAM_LOWPAN;
					if (type_name == "dgram:ethernet") type = Configuration::NetConfig::NETWORK_DGRAM_ETHERNET;

					switch (type)
					{
						case Configuration::NetConfig::NETWORK_HTTP:
						{
							Configuration::NetConfig nc(netname, type,
									conf.read<std::string>(key_address, "http://localhost/"),
									conf.read<std::string>(key_discovery, "yes") == "no");

							_interfaces.push_back(nc);
							break;
						}

						case Configuration::NetConfig::NETWORK_FILE:
						{
							Configuration::NetConfig nc(netname, type,
									conf.read<std::string>(key_path, ""),
									conf.read<std::string>(key_discovery, "yes") == "no");

							_interfaces.push_back(nc);
							break;
						}

						default:
						{
							int port = conf.read<int>(key_port, 4556);
							bool discovery = (conf.read<std::string>(key_discovery, "yes") == "yes");
							int mtu = conf.read<int>(key_mtu, 1280);

							try {
								ibrcommon::vinterface interface(conf.read<std::string>(key_interface));
								Configuration::NetConfig nc(netname, type, interface, port, discovery);
								nc.mtu = mtu;
								_interfaces.push_back(nc);
							} catch (const ConfigFile::key_not_found&) {
								ibrcommon::vaddress addr;
								Configuration::NetConfig nc(netname, type, addr, port, discovery);
								nc.mtu = mtu;
								_interfaces.push_back(nc);
							}

							break;
						}
					}
				}
			} catch (const ConfigFile::key_not_found&) {
				// stop the one network is not found.
			}

			/**
			 * TCP options
			 */
			_tcp_nodelay = (conf.read<std::string>("tcp_nodelay", "yes") == "yes");
			_tcp_chunksize = conf.read<unsigned int>("tcp_chunksize", 4096);
			_tcp_idle_timeout = conf.read<unsigned int>("tcp_idle_timeout", 0);

			/**
			 * dynamic rebind
			 */
			_dynamic_rebind = (conf.read<std::string>("net_rebind", "no") == "yes");

			/**
			 * auto connect interval
			 */
			_auto_connect = conf.read<size_t>("net_autoconnect", 0);

			/**
			 * fragmentation support
			 */
			_fragmentation = (conf.read<std::string>("fragmentation", "no") == "yes");
		}

		const std::multimap<std::string, std::string>& Configuration::Network::getStaticRoutes() const
		{
			return _static_routes;
		}

		const std::list<Node>& Configuration::Network::getStaticNodes() const
		{
			return _nodes;
		}

		int Configuration::getTimezone()
		{
			return _conf.read<int>( "timezone", 0 );
		}

		ibrcommon::File Configuration::getPath(string name)
		{
			stringstream ss;
			ss << name << "_path";
			string key; ss >> key;

			try {
				return ibrcommon::File(_conf.read<string>(key));
			} catch (const ConfigFile::key_not_found&) {
				throw ParameterNotSetException();
			}
		}

		const std::string Configuration::getUser() const
		{
			try {
				return _conf.read<std::string>("user");
			} catch (const ConfigFile::key_not_found&) {
				throw ParameterNotSetException();
			}
		}

		unsigned int Configuration::getUID() const
		{
			try {
				return _conf.read<unsigned int>("uid");
			} catch (const ConfigFile::key_not_found&) {
				throw ParameterNotSetException();
			}
		}

		unsigned int Configuration::getGID() const
		{
			try {
				return _conf.read<unsigned int>("gid");
			} catch (const ConfigFile::key_not_found&) {
				throw ParameterNotSetException();
			}
		}


		bool Configuration::Discovery::enabled() const
		{
			return _enabled;
		}

		bool Configuration::Discovery::announce() const
		{
			return (Configuration::getInstance()._conf.read<int>("discovery_announce", 1) == 1);
		}

		bool Configuration::Discovery::shortbeacon() const
		{
			return (Configuration::getInstance()._conf.read<int>("discovery_short", 0) == 1);
		}

		char Configuration::Discovery::version() const
		{
			return Configuration::getInstance()._conf.read<int>("discovery_version", 2);
		}

		bool Configuration::doAPI()
		{
			return _doapi;
		}

		string Configuration::getNotifyCommand()
		{
			try {
				return _conf.read<string>("notify_cmd");
			} catch (const ConfigFile::key_not_found&) {
				throw ParameterNotSetException();
			}
		}

		Configuration::RoutingExtension Configuration::Network::getRoutingExtension() const
		{
			if ( _routing == "epidemic" ) return EPIDEMIC_ROUTING;
			if ( _routing == "flooding" ) return FLOOD_ROUTING;
			if ( _routing == "prophet" ) return PROPHET_ROUTING;
			return DEFAULT_ROUTING;
		}


		bool Configuration::Network::doForwarding() const
		{
			return _forwarding;
		}

		bool Configuration::Network::doFragmentation() const
		{
			return _fragmentation;
		}

		bool Configuration::Network::getTCPOptionNoDelay() const
		{
			return _tcp_nodelay;
		}

		size_t Configuration::Network::getTCPChunkSize() const
		{
			return _tcp_chunksize;
		}

		size_t Configuration::Network::getTCPIdleTimeout() const
		{
			return _tcp_idle_timeout;
		}

		bool Configuration::Network::doDynamicRebind() const
		{
			return _dynamic_rebind;
		}

		size_t Configuration::Network::getAutoConnect() const
		{
			return _auto_connect;
		}

		Configuration::Network::ProphetConfig Configuration::Network::getProphetConfig() const
		{
			return _prophet_config;
		}


		bool Configuration::Statistic::enabled() const
		{
			return Configuration::getInstance()._conf.keyExists("statistic_type");
		}

		ibrcommon::File Configuration::Statistic::logfile() const throw (ParameterNotSetException)
		{
			try {
				return ibrcommon::File(Configuration::getInstance()._conf.read<std::string>("statistic_file"));
			} catch (const ConfigFile::key_not_found&) {
				throw ParameterNotSetException();
			}
		}

		std::string Configuration::Statistic::type() const
		{
			return Configuration::getInstance()._conf.read<std::string>("statistic_type", "stdout");
		}

		unsigned int Configuration::Statistic::interval() const
		{
			return Configuration::getInstance()._conf.read<unsigned int>("statistic_interval", 300);
		}

		std::string Configuration::Statistic::address() const
		{
			return Configuration::getInstance()._conf.read<std::string>("statistic_address", "127.0.0.1");
		}

		unsigned int Configuration::Statistic::port() const
		{
			return Configuration::getInstance()._conf.read<unsigned int>("statistic_port", 1234);
		}

		size_t Configuration::getLimit(std::string suffix)
		{
			std::string unparsed = _conf.read<std::string>("limit_" + suffix, "0");

			std::stringstream ss(unparsed);

			float value; ss >> value;
			char multiplier = 0; ss >> multiplier;

			switch (multiplier)
			{
			default:
				return (size_t)value;
				break;

			case 'G':
				return (size_t)(value * 1000000000);
				break;

			case 'M':
				return (size_t)(value * 1000000);
				break;

			case 'K':
				return (size_t)(value * 1000);
				break;
			}

			return 0;
		}

		void Configuration::Security::load(const ibrcommon::ConfigFile &conf)
		{
			bool withTLS = false;
#ifdef WITH_TLS
			withTLS = true;
			/* enable TLS if the certificate path, a certificate and the private key was given */
			bool activateTLS = true;

			// load public key file
			try {
				_cert = conf.read<std::string>("security_certificate");

				if (!_cert.exists())
				{
					IBRCOMMON_LOGGER(warning) << "Certificate file " << _cert.getPath() << " does not exists!" << IBRCOMMON_LOGGER_ENDL;
					activateTLS = false;
				}
			} catch (const ibrcommon::ConfigFile::key_not_found&) {
				activateTLS = false;
			}

			// load KEY file
			try {
				_key = conf.read<std::string>("security_key");

				if (!_key.exists())
				{
					IBRCOMMON_LOGGER(warning) << "KEY file " << _key.getPath() << " does not exists!" << IBRCOMMON_LOGGER_ENDL;
					activateTLS = false;
				}
			} catch (const ibrcommon::ConfigFile::key_not_found&) {
				activateTLS = false;
			}

			// read trustedCAPath
			try{
				_trustedCAPath = conf.read<std::string>("security_trusted_ca_path");
				if(!_trustedCAPath.isDirectory()){
					IBRCOMMON_LOGGER(warning) << "Trusted CA Path " << _trustedCAPath.getPath() << " does not exists or is no directory!" << IBRCOMMON_LOGGER_ENDL;
					activateTLS = false;
				}
			} catch (const ibrcommon::ConfigFile::key_not_found&) {
				activateTLS = false;
			}

			// read if encryption should be disabled
			_disableEncryption = (conf.read<std::string>("security_tls_disable_encryption", "no") == "yes");

			if (activateTLS)
			{
				_tlsEnabled = true;

				/* read if TLS is required */
				_tlsRequired = (conf.read<std::string>("security_tls_required", "no") == "yes");

				/* If the clock is not in sync, SSL will fail. Accept plain connection when the clock is not sync'ed. */
				_tlsOptionalOnBadClock = (conf.read<std::string>("security_tls_fallback_badclock", "no") == "yes");
			}
#endif

#ifdef WITH_BUNDLE_SECURITY
			// enable security if the security path is set
			try {
				_path = conf.read<std::string>("security_path");

				if (!_path.exists())
				{
					ibrcommon::File::createDirectory(_path);
				}

				_enabled = true;
			} catch (const ibrcommon::ConfigFile::key_not_found&) {
				return;
			}

			// load level
			_level = Level(conf.read<int>("security_level", 0));

			if ( !withTLS )
			{
				/* if TLS is enabled, the Certificate file and the key have been read earlier */
				// load CA file
				try {
					_cert = conf.read<std::string>("security_certificate");

					if (!_cert.exists())
					{
						IBRCOMMON_LOGGER(warning) << "Certificate file " << _cert.getPath() << " does not exists!" << IBRCOMMON_LOGGER_ENDL;
					}
				} catch (const ibrcommon::ConfigFile::key_not_found&) { }

				// load KEY file
				try {
					_key = conf.read<std::string>("security_key");

					if (!_key.exists())
					{
						IBRCOMMON_LOGGER(warning) << "KEY file " << _key.getPath() << " does not exists!" << IBRCOMMON_LOGGER_ENDL;
					}
				} catch (const ibrcommon::ConfigFile::key_not_found&) { }
			}

			// load KEY file
			try {
				_bab_default_key = conf.read<std::string>("security_bab_default_key");

				if (!_bab_default_key.exists())
				{
					IBRCOMMON_LOGGER(warning) << "KEY file " << _bab_default_key.getPath() << " does not exists!" << IBRCOMMON_LOGGER_ENDL;
				}
			} catch (const ibrcommon::ConfigFile::key_not_found&) {
			}
#endif
		};

		Configuration::Security::~Security() {};

		bool Configuration::Security::enabled() const
		{
			return _enabled;
		}

		bool Configuration::Security::doTLS() const
		{
			return _tlsEnabled;
		}

		bool Configuration::Security::TLSRequired() const
		{
			// TLS is only required, if the clock is in sync
			if ((dtn::utils::Clock::rating == 0) && _tlsOptionalOnBadClock) return false;
			return _tlsRequired;
		}

#ifdef WITH_BUNDLE_SECURITY
		const ibrcommon::File& Configuration::Security::getPath() const
		{
			return _path;
		}

		Configuration::Security::Level Configuration::Security::getLevel() const
		{
			return _level;
		}

		const ibrcommon::File& Configuration::Security::getBABDefaultKey() const
		{
			return _bab_default_key;
		}
#endif
#if defined WITH_BUNDLE_SECURITY || defined WITH_TLS
		const ibrcommon::File& Configuration::Security::getCertificate() const
		{
			return _cert;
		}

		const ibrcommon::File& Configuration::Security::getKey() const
		{
			return _key;
		}
#endif
#ifdef WITH_TLS
		const ibrcommon::File& Configuration::Security::getTrustedCAPath() const
		{
			return _trustedCAPath;
		}

		bool Configuration::Security::TLSEncryptionDisabled() const
		{
			return _disableEncryption;
		}
#endif

		bool Configuration::Logger::quiet() const
		{
			return _quiet;
		}

		const ibrcommon::File& Configuration::Logger::getLogfile() const
		{
			if (_logfile.getPath() == "") throw Configuration::ParameterNotSetException();
			return _logfile;
		}

		bool Configuration::Logger::display_timestamps() const
		{
			return _timestamps;
		}

		unsigned int Configuration::Logger::options() const
		{
			return _options;
		}

		std::ostream& Configuration::Logger::output() const
		{
			return std::cout;
		}

		bool Configuration::Daemon::daemonize() const
		{
			return _daemonize;
		}

		bool Configuration::Daemon::kill_daemon() const
		{
			return _kill;
		}

		size_t Configuration::Daemon::getThreads() const
		{
			return _threads;
		}

		const ibrcommon::File& Configuration::Daemon::getPidFile() const
		{
			if (_pidfile == ibrcommon::File()) throw ParameterNotSetException();
			return _pidfile;
		}

		bool Configuration::TimeSync::hasReference() const
		{
			return _reference;
		}

		bool Configuration::TimeSync::syncOnDiscovery() const
		{
			return _sync;
		}

		bool Configuration::TimeSync::sendDiscoveryAnnouncements() const
		{
			return _discovery;
		}

		float Configuration::TimeSync::getSigma() const
		{
			return _sigma;
		}

		float Configuration::TimeSync::getPsi() const
		{
			return _psi;
		}

		float Configuration::TimeSync::getSyncLevel() const
		{
			return _sync_level;
		}

		bool Configuration::DHT::enabled() const
		{
			return _enabled;
		}

		bool Configuration::DHT::randomPort() const
		{
			return _port == 0;
		}

		unsigned int Configuration::DHT::getPort() const
		{
			return _port;
		}

		string Configuration::DHT::getID() const
		{
			return _id;
		}

		bool Configuration::DHT::randomID() const
		{
			return _id == "";
		}

		bool Configuration::DHT::isDNSBootstrappingEnabled() const
		{
			return _dnsbootstrapping;
		}

		std::vector<string> Configuration::DHT::getDNSBootstrappingNames() const
		{
			return _bootstrappingdomains;
		}

		bool Configuration::DHT::isIPBootstrappingEnabled() const
		{
			return _bootstrappingips.size() > 0;
		}

		std::vector<string> Configuration::DHT::getIPBootstrappingIPs() const
		{
			return _bootstrappingips;
		}

		string Configuration::DHT::getIPv4Binding() const
		{
			return _ipv4bind;
		}
		string Configuration::DHT::getIPv6Binding() const
		{
			return _ipv6bind;
		}

		string Configuration::DHT::getPathToNodeFiles() const
		{
			return _nodesFilePath;
		}

		bool Configuration::DHT::isIPv4Enabled() const
		{
			return _ipv4;
		}

		bool Configuration::DHT::isIPv6Enabled() const
		{
			return _ipv6;
		}

		bool Configuration::DHT::isSelfAnnouncingEnabled() const
		{
			return _selfannounce;
		}

		int Configuration::DHT::getMinimumRating() const
		{
			return _minRating;
		}

		bool Configuration::DHT::isNeighbourAnnouncementEnabled() const
		{
			return _allowNeighbourAnnouncement;
		}

		bool Configuration::DHT::isNeighbourAllowedToAnnounceMe() const
		{
			return _allowNeighbourToAnnounceMe;
		}

		bool Configuration::DHT::isBlacklistEnabled() const
		{
			return _blacklist;
		}
		
		bool Configuration::DHT::ignoreDHTNeighbourInformations() const
		{
			return _ignoreDHTNeighbourInformations;
		}
	}
}
