
noinst_HEADERS = \
	ConfigurationTest.hh \
	BaseRouterTest.hh \
	SimpleBundleStorageTest.hh \
	DataStorageTest.h \
	NodeTest.hh \
	DaemonTest.hh
	
#	UDPConvergenceLayerTest.hh \
#	SQLiteBundleStorageTest.hh \
#	LOWPANConvergenceLayerTest.hh \
#	NeighborRoutingExtensionTest.hh \
#	HTTPConvergenceLayerTest.hh \
#	TimeEventTest.hh \
#	TCPConvergenceLayerTest.hh \
#	ConnectionEventTest.hh \
#	BundleExpiredEventTest.hh \
#	EchoWorkerTest.hh \
#	NodeEventTest.hh \
#	ClientHandlerTest.hh \
#	CustodyEventTest.hh \
#	FloodRoutingExtensionTest.hh \
#	EventDebuggerTest.hh \
#	WallClockTest.hh \
#	BundleStorageTest.hh \
#	GlobalEventTest.hh \
#	DiscoveryAgentTest.hh \
#	QueueBundleEventTest.hh \
#	DevNullTest.hh \
#	ComponentTest.hh \
#	EpidemicRoutingExtensionTest.hh \
#	StaticRoutingExtensionTest.hh \
#	EventTest.hh \
#	BundleCoreTest.hh \
#	SQLiteConfigureTest.hh \
#	StatusReportGeneratorTest.hh \
#	DiscoveryAnnouncementTest.hh \
#	TransferCompletedEventTest.hh \
#	EventReceiverTest.hh \
#	RequeueBundleEventTest.hh \
#	BundleEventTest.hh \
#	RetransmissionExtensionTest.hh \
#	DiscoveryServiceTest.hh \
#	BundleReceivedEventTest.hh \
#	DebuggerTest.hh \
#	NeighborDatabaseTest.hh \
#	TransferAbortedEventTest.hh \
#	ConnectionManagerTest.hh \
#	BundleGeneratedEventTest.hh \
#	AbstractWorkerTest.hh \
#	IPNDAgentTest.hh \
#	EventSwitchTest.hh

unittest_SOURCES = \
	Main.cpp \
	BaseRouterTest.cpp \
	ConfigurationTest.cpp \
	SimpleBundleStorageTest.cpp \
	DataStorageTest.cpp \
	NodeTest.cpp \
	DaemonTest.cpp
	
#	UDPConvergenceLayerTest.cpp \
#	SQLiteBundleStorageTest.cpp \
#	LOWPANConvergenceLayerTest.cpp \
#	NeighborRoutingExtensionTest.cpp \
#	HTTPConvergenceLayerTest.cpp \
#	TimeEventTest.cpp \
#	TCPConvergenceLayerTest.cpp \
#	ConnectionEventTest.cpp \
#	BundleExpiredEventTest.cpp \
#	EchoWorkerTest.cpp \
#	NodeEventTest.cpp \
#	ClientHandlerTest.cpp \
#	CustodyEventTest.cpp \
#	FloodRoutingExtensionTest.cpp \
#	EventDebuggerTest.cpp \
#	WallClockTest.cpp \
#	BundleStorageTest.cpp \
#	GlobalEventTest.cpp \
#	DiscoveryAgentTest.cpp \
#	QueueBundleEventTest.cpp \
#	DevNullTest.cpp \
#	ComponentTest.cpp \
#	EpidemicRoutingExtensionTest.cpp \
#	StaticRoutingExtensionTest.cpp \
#	EventTest.cpp \
#	BundleCoreTest.cpp \
#	SQLiteConfigureTest.cpp \
#	StatusReportGeneratorTest.cpp \
#	DiscoveryAnnouncementTest.cpp \
#	TransferCompletedEventTest.cpp \
#	EventReceiverTest.cpp \
#	RequeueBundleEventTest.cpp \
#	BundleEventTest.cpp \
#	RetransmissionExtensionTest.cpp \
#	DiscoveryServiceTest.cpp \
#	BundleReceivedEventTest.cpp \
#	DebuggerTest.cpp \
#	NotifierTest.cpp \
#	StatisticLoggerTest.cpp \
#	NeighborDatabaseTest.cpp \
#	TransferAbortedEventTest.cpp \
#	ConnectionManagerTest.cpp \
#	BundleGeneratedEventTest.cpp \
#	AbstractWorkerTest.cpp \
#	IPNDAgentTest.cpp \
#	EventSwitchTest.cpp

# what flags you want to pass to the C compiler & linker
AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/tests/unittests $(ibrdtn_CFLAGS) $(CPPUNIT_CFLAGS) -Wall $(CURL_CFLAGS) $(SQLITE_CFLAGS)
AM_LDFLAGS = $(ibrdtn_LIBS) $(CPPUNIT_LIBS) $(CURL_LIBS) $(SQLITE_LIBS)

check_PROGRAMS = unittest
unittest_CXXFLAGS = $(AM_CPPFLAGS) $(CPPUNIT_CFLAGS) -Wall
unittest_LDFLAGS = $(AM_LDFLAGS) $(CPPUNIT_LIBS)
unittest_LDADD = $(top_srcdir)/src/libdtnd.la

TESTS = unittest
