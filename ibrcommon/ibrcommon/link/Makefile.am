## sub directory

h_sources = nl802154.h LinkManager.h LinkEvent.h PosixLinkManager.h
cc_sources = LinkManager.cpp LinkEvent.cpp PosixLinkManager.cpp

if LOWPAN
cc_sources += nl_policy.c
endif

if NETLINK
h_sources += NetLinkManager.h
cc_sources += NetLinkManager.cpp
endif

#Install the headers in a versioned directory
library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)/link
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(PTHREAD_CFLAGS) $(NL_CFLAGS) $(NL_ROUTE_CFLAGS) $(NL_GENL_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS) $(NL_LIBS) $(NL_ROUTE_LIBS) $(NL_GENL_LIBS)

noinst_LTLIBRARIES= liblink.la
liblink_la_SOURCES= $(h_sources) $(cc_sources)
