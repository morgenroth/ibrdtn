## sub directory

h_sources = socket.h socketstream.h ieee802154.h nl802154.h stopandwait.h vsocket.h vinterface.h vaddress.h LinkManager.h
cc_sources = socket.cpp socketstream.h stopandwait.cpp vsocket.cpp vinterface.cpp vaddress.cpp LinkManager.cpp

if LOWPAN
h_sources += lowpansocket.h lowpanstream.h
cc_sources += lowpansocket.cpp nl_policy.c lowpanstream.cpp
endif

if NETLINK
h_sources += NetLinkManager.h
cc_sources += NetLinkManager.cpp
endif

if NETLINK3
h_sources += NetLink3Manager.h
cc_sources += NetLink3Manager.cpp
endif

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/net
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(PTHREAD_CFLAGS) $(NL_CFLAGS) $(NL_ROUTE_CFLAGS) $(NL_GENL_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS) $(NL_LIBS) $(NL_ROUTE_LIBS) $(NL_GENL_LIBS)

noinst_LTLIBRARIES= libnet.la
libnet_la_SOURCES= $(h_sources) $(cc_sources)
