## sub directory

AM_CPPFLAGS = -I$(top_srcdir) @ibrdtn_CFLAGS@
AM_LDFLAGS = @ibrdtn_LIBS@

net_SOURCES = 	BundleReceivedEvent.cpp \
				BundleReceivedEvent.h \
				BundleReceiver.h \
				ConnectionEvent.cpp \
				ConnectionEvent.h \
				ConnectionManager.cpp \
				ConnectionManager.h \
				ConvergenceLayer.cpp \
				ConvergenceLayer.h \
				DiscoveryAgent.cpp \
				DiscoveryAgent.h \
				DiscoveryAnnouncement.cpp \
				DiscoveryAnnouncement.h \
				DiscoveryService.cpp \
				DiscoveryService.h \
				DiscoveryServiceProvider.h \
				IPNDAgent.cpp \
				IPNDAgent.h \
				Neighbor.cpp \
				Neighbor.h \
				TCPConnection.cpp \
				TCPConnection.h \
				TCPConvergenceLayer.cpp \
				TCPConvergenceLayer.h \
				TransferAbortedEvent.cpp \
				TransferAbortedEvent.h \
				TransferCompletedEvent.cpp \
				TransferCompletedEvent.h \
				UDPConvergenceLayer.cpp \
				UDPConvergenceLayer.h \
				FileConvergenceLayer.cpp \
				FileConvergenceLayer.h \
				DatagramConnectionParameter.h \
				DatagramConvergenceLayer.h \
				DatagramConnection.h \
				DatagramConvergenceLayer.cpp \
				DatagramConnection.cpp \
				UDPDatagramService.h \
				UDPDatagramService.cpp \
				FileMonitor.h \
				FileMonitor.cpp

if LOWPAN
net_SOURCES += LOWPANConvergenceLayer.cpp LOWPANConvergenceLayer.h LOWPANConnection.cpp LOWPANConnection.h
net_SOURCES += LOWPANDatagramService.cpp LOWPANDatagramService.h
endif
				
if CURL
net_SOURCES += HTTPConvergenceLayer.cpp HTTPConvergenceLayer.h
AM_CPPFLAGS += @CURL_CFLAGS@
AM_LDFLAGS += @CURL_LIBS@
endif

if DTNDHT
net_SOURCES += DHTNameService.cpp DHTNameService.h
AM_CPPFLAGS += @DTNDHT_CFLAGS@
AM_LDFLAGS += @DTNDHT_LIBS@
endif

noinst_LTLIBRARIES = libnet.la
libnet_la_SOURCES= $(net_SOURCES)
