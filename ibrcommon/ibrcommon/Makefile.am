## Source directory

SUBDIRS = data net thread

if OPENSSL
  SUBDIRS += ssl
endif

if LIBXML
  SUBDIRS += xml
endif

noinst_HEADERS = config.h

h_sources = ibrcommon.h appstreambuf.h Exceptions.h refcnt_ptr.h SyslogStream.h TimeMeasurement.h Logger.h TLSExceptions.h
cc_sources = appstreambuf.cpp SyslogStream.cpp TimeMeasurement.cpp Logger.cpp

library_includedir=$(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

AM_CPPFLAGS = -I$(top_srcdir) $(PTHREAD_CFLAGS) $(NL_CFLAGS) $(NL_ROUTE_CFLAGS)
AM_LDFLAGS = $(PTHREAD_LIBS) $(NL_LIBS) $(NL_ROUTE_LIBS)

lib_LTLIBRARIES= libibrcommon.la
libibrcommon_la_SOURCES= $(h_sources) $(cc_sources)
libibrcommon_la_LIBADD= data/libdata.la net/libnet.la thread/libthread.la
libibrcommon_la_LDFLAGS= -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)

if OPENSSL
  libibrcommon_la_LIBADD += ssl/libibrcommonssl.la ssl/gcm/libgcm.la
endif

if LIBXML
  libibrcommon_la_LIBADD += xml/libibrcommonxml.la
endif
